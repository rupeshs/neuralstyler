/*
    NeuralStyler,Artistic style for your videos/photos
    Copyright(C) 2016 Rupesh Sreeraman
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "neuralstylerwindow.h"
#include "ui_neuralstylerwindow.h"

NeuralStylerWindow::NeuralStylerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NeuralStylerWindow)
{

    ui->setupUi(this);
    // QCoreApplication::addLibraryPath(qApp->applicationDirPath()+QDir::separator()+"platforms");
    paths=new Paths(qApp->applicationDirPath());
    QDir styleDir(paths->getStylePath());
    QStringList filters;
    QStringList lstStyles;
    ui->checkBoxKeepAspectRatio->setChecked(true);

    filters<<"*.model";
    styleDir.setNameFilters(filters);
    lstStyles.clear();
    lstStyles=styleDir.entryList();
    QStringList lstStyleNames;
    for (int i = 0; i < lstStyles.count(); i++) {
        QFileInfo fileInfo(QFile(paths->getStylePath()+lstStyles.at(i)));
        lstStyleNames<<fileInfo.baseName();
    }
    ui->comboBoxStyles->addItems(lstStyleNames);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));

    // m_mpv = new MpvWidget(this);
#ifdef Q_OS_WIN
    mPath= qApp->applicationDirPath()+ "/mplayer/mplayer.exe";
#endif
#ifdef Q_OS_LINUX
    mPath="/usr/bin/mplayer";
# endif
#ifdef Q_OS_OPENBSD
    mPath="/usr/local/bin/mplayer";
# endif
    if(!QFile(mPath).exists()){
        QMessageBox::critical(this,qApp->applicationName(),"Oops..MPlayer not found,Video trimming will not work...Please install MPlayer");
    }

    qDebug()<<mPath;
    mp=new Mplayer();
    mp->setMplayerPath(mPath);
    connect(mp,SIGNAL(gotduration(float)),this,SLOT(setSliderRange(float)));
    connect(ui->frameSlider, SIGNAL(lowerValueChanged(int)), SLOT(seek(int)));
    connect(ui->frameSlider, SIGNAL(upperValueChanged(int)), SLOT(seek(int)));

    mplayerVideo=new QLabel(this);
    mplayerVideo->setGeometry(0,0,261,181);
    mplayerVideo->setMinimumWidth(261);


    mplayerVideo->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
    mplayerVideo->setStyleSheet("background-color:black;");
    mp->setVideoWin(mplayerVideo->winId());
#ifdef Q_OS_LINUX
    mp->setScaling(261,181);
#endif

            QVBoxLayout *vl = new QVBoxLayout();

    videoTimeLabel = new QLabel("00:00:00");
    videoStatusLabel = new QLabel("Select video section");

    vl->addWidget(mplayerVideo);
    vl->addWidget(ui->frameSlider);
    vl->addWidget(videoStatusLabel);
    vl->addWidget(videoTimeLabel);
    ui->groupBox_2->setLayout(vl);

    ui->frameSlider->setHandleMovementMode(QxtSpanSlider::NoCrossing);
    ui->pushButtonPause->setEnabled(false);
    ui->pushButtonViewFrame->setEnabled(false);

    //Check settings exists
    if(!QFile(qApp->applicationDirPath()+"/neuralstyler.ini").exists()){
        qDebug()<<"Settings not found!";
        paths->setStyledVideoPath(qApp->applicationDirPath()+"/styledvideo");
        ui->lineEditSavePath->setText( paths->getStyledVideoPath());

    }

    settings=new QSettings(qApp->applicationDirPath()+"/neuralstyler.ini",QSettings::IniFormat,this);


    qDebug()<<settings->value("Main/destination","").toString();
    if(settings->value("Main/destination","").toString().compare("")==0){

        settings->beginGroup("Main");
        settings->setValue("destination", paths->getStyledVideoPath());
        settings->endGroup();


    }
    else{
        ui->lineEditSavePath->setText(settings->value("Main/destination",paths->getStyledVideoPath()).toString());
        paths->setStyledVideoPath(ui->lineEditSavePath->text());
    }

    ui->groupBox_2->setEnabled(false);
    ui->groupBoxRes->setVisible(false);

    bitSettingsFirstRun=new QBitArray(2,true);
    //bit 0 first run
    //bit 1 load trim slider
    engineState(false);
    setAcceptDrops(true);



}

NeuralStylerWindow::~NeuralStylerWindow()
{
    delete ui;
}

void NeuralStylerWindow::on_pushButonBrowseFile_clicked()
{

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open video"), "", tr("videoFiles (*.*)"));


    if (fileName.isEmpty())
        return;
    ui->lineEditFilePath->setText(fileName);
    initVideoTrim();

}

void NeuralStylerWindow::on_pushButtonProcess_clicked()
{

    if (ui->pushButtonProcess->text().compare("Cancel Neural Styling")==0){

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "NeuralStyler", "Do you want to stop styling ?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            engineState(true);
            if (ffmpeg){
                ffmpeg->write("q\n");
                ffmpeg->waitForBytesWritten(1000);
            }
            qApp->exit( NeuralStylerWindow::EXIT_CODE_REBOOT );
        }
        else{
            return;
        }
    }

    // qApp->exit( NeuralStylerWindow::EXIT_CODE_REBOOT );
    tElapsed.start();
    timer->start(1000);

    if (!QFile(ui->lineEditFilePath->text()).exists()){
        QMessageBox::critical(this,qApp->applicationName(),"Please provide a valid file!");
        return;
    }
    if(ui->comboBoxResolution->currentText().compare("Custom")==0){

        if (ui->lineEditWidth->text().toInt()>0&&ui->lineEditHeight->text().toInt())
        {
            //process
        }else{
            QMessageBox::critical(this,qApp->applicationName(),"Please provide valid resolution(Width x Height)");
            return;

        }
    }
    //ui->pushButtonProcess->setText("Cancel");
    ui->progressBarOverall->setValue(0);
    ui->groupBox->setDisabled(true);
    ui->groupBox_2->setDisabled(true);

    //Clean folders
    QDir(paths->getAudioPath()).removeRecursively();
    QDir(paths->getFramePath()).removeRecursively();
    QDir(paths->getStyledFramePath()).removeRecursively();

    paths->ensureFramePath();
    paths->ensureAudioPath();
    paths->ensureStyledFramePath();
    paths->ensureStyledVideoPath();
    engineState(true);
    getFramePerSec();
    ui->pushButtonProcess->setText("Cancel Neural Styling");
    ui->pushButtonProcess->setIcon(QPixmap(":/images/paint-cancel.png"));

}
void NeuralStylerWindow::fpsStarted()
{
    qDebug()<<"Get fps";

}
void NeuralStylerWindow::fpsCompleted(int ec)
{
    ui->labelStatus->setText("Extracting frames...");
    strFramesPerSec=ffmpeg->getFps();
    qDebug()<<"fps ok "<<strFramesPerSec;
    strDuration=ffmpeg->getDuration();

    QTime totTime=QTime::fromString(strDuration.trimmed(),"hh:mm:ss.z");
    //  qDebug()<<"Video Duration :"<< strDuration;
    qDebug()<<"Video Duration :"<<totTime.toString();
    // totTime.secsTo();
    int durationSec=QTime(0,0,0).secsTo(totTime);

    frameCount=0;
    frameCount=strFramesPerSec.toFloat()* durationSec;
    qDebug()<<"frames:"<<QString::number(frameCount);

    ui->progressBarFrame->setValue(0);
    ffmpeg->deleteLater();

    ui->progressBarOverall->setValue(16);
    extractFrames();

}

void NeuralStylerWindow::getFramePerSec()
{
    strFramesPerSec="";
    ffmpeg=new FfmpegProcess();
    connect(ffmpeg,SIGNAL(started()),this,SLOT(fpsStarted()));
    connect(ffmpeg,SIGNAL(finished(int)),this,SLOT(fpsCompleted(int)));

    QStringList arguments;
    arguments << "-i" << ui->lineEditFilePath->text();

    ffmpeg->setArguments(arguments);
    ffmpeg->startFfmpeg();
}
void NeuralStylerWindow::extractFrames()
{
    paths->ensureFramePath();
    ui->progressBarOverall->setValue(33);
    //./ffmpeg -i strip_jp.mkv -vf scale=320:240 frames/frame%05d.jpg
    ffmpeg=new FfmpegProcess();
    connect(ffmpeg,SIGNAL(started()),this,SLOT(frameExtratorStarted()));
    connect(ffmpeg,SIGNAL(finished(int)),this,SLOT(frameExtratorCompleted(int)));
    connect(ffmpeg,SIGNAL(gotFrame(QString)),this,SLOT(gotFrame(QString)));
    QStringList arguments;
    QString strResolution=ui->comboBoxResolution->currentText();

    if (strResolution.compare("Custom")==0) {
        strResolution=ui->lineEditWidth->text()+"x"+ui->lineEditHeight->text();
    }

    QString strWidth;
    QString strHeight;

    if (rx_width.indexIn(strResolution)>-1) {

        strWidth=rx_width.cap(1);
    }
    if (rx_height.indexIn(strResolution)>-1) {

        strHeight=rx_height.cap(1);
    }

    //<<"-ss"<<QString::number(startPos)<<"-t"<<QString::number(stopPos-startPos)
    QString strStartPos=QString::number(ui->frameSlider->lowerValue());
    QString strStyleDur=QString::number(ui->frameSlider->upperValue()-ui->frameSlider->lowerValue());

    //input
    arguments << "-i" << ui->lineEditFilePath->text() ;

    //trim video
    if (!isImageOrGif())
        arguments<<"-ss"<<strStartPos<<"-t"<<strStyleDur;

    //Scale image
    if (!ui->checkBoxNoScale->isChecked()){

        //aspect ratio
        if(ui->checkBoxKeepAspectRatio->isChecked())
            arguments <<"-vf"<<"scale="+strWidth+":"+QString::number(-1)+":flags=lanczos";
        else
            arguments<<"-vf"<<"scale="+strResolution.replace("x",":")+":flags=lanczos";
    }

    //output
    arguments <<"-qscale:v"<<"2"<<paths->getFramePath()+"frame%05d.jpg";

    ffmpeg->setArguments(arguments);
    ffmpeg->startFfmpeg();

}
void NeuralStylerWindow::frameExtratorStarted()
{
    qDebug()<<"Extract frames";

}
void NeuralStylerWindow::frameExtratorCompleted(int ec)
{
    qDebug()<<"Frame extraction ok";
    //delete ffmpeg;
    ffmpeg->deleteLater();
    ui->progressBarOverall->setValue(49);
    ui->progressBarFrame->setValue(100);
    ui->labelStatus->setText("Extracting audio..");
    extractAudio();

}
void NeuralStylerWindow::extractAudio()
{
    ffmpeg=new FfmpegProcess();
    connect(ffmpeg,SIGNAL(started()),this,SLOT(audioExtratorStarted()));
    connect(ffmpeg,SIGNAL(finished(int)),this,SLOT(audioExtratorCompleted(int)));

    QStringList arguments;
    QString strStartPos=QString::number(ui->frameSlider->lowerValue());
    QString strStyleDur=QString::number(ui->frameSlider->upperValue()-ui->frameSlider->lowerValue());


    arguments << "-i" << ui->lineEditFilePath->text()<<"-ss"<<strStartPos<<"-t"<<strStyleDur<<"-y"<<paths->getAudioPath()+"audio.wav";
    ffmpeg->setArguments(arguments);
    ffmpeg->startFfmpeg();

}

void NeuralStylerWindow::audioExtratorStarted()
{
    qDebug()<<"Extract audio";

}
void NeuralStylerWindow::audioExtratorCompleted(int ec)
{
    qDebug()<<"Audio extraction ok";
    ui->progressBarOverall->setValue(65);
    //delete ffmpeg;
    ffmpeg->deleteLater();
    ui->labelStatus->setText("Styling frames..");
    paths->ensureStyledFramePath();
    styleFrames();

}

void NeuralStylerWindow::styleFrames()
{

    //ui->pushButtonViewFrame->setEnabled(true);
    QDir framesDir(paths->getFramePath());

    QStringList filters;
    filters<<"*.jpg";
    framesDir.setNameFilters(filters);
    lstFrames.clear();
    lstFrames=framesDir.entryList();
    //qDebug()<<lstFrames;

    //python generate.py rup.jpeg -m models/seurat.model -o rupstyle2.jpg
    processingFrameCount=0;
    if(lstFrames.count()>0)
        chainerProcess();
    else
    {
        QMessageBox::critical(this,qApp->applicationName(),"Styling failed! Invalid file.");
        finishStyling();

    }
    if(lstFrames.size()>1){
        ui->pushButtonViewFrame->setEnabled(true);
        ui->pushButtonPause->setEnabled(true);
    }

}
void NeuralStylerWindow::fastNeuralStyleStarted()
{
    qDebug()<<"Style start";

}
void NeuralStylerWindow::fastNeuralStyleCompleted(int ec)
{

    if (fastNeuralStyle->getProcessingTime().isEmpty()){
        qDebug()<<"Style Frame failed";
        QMessageBox::critical(this,qApp->applicationName(),"Memory Error:Insufficient RAM to process this resolution,change resolution");
        timer->stop();
        fastNeuralStyle->deleteLater();
        qApp->exit( NeuralStylerWindow::EXIT_CODE_REBOOT );

        return;

    }
    else
    {
        QImage top(paths->getStyledFramePath()+lstFrames.at(processingFrameCount-1));
        QImage bot(paths->getFramePath()+lstFrames.at(processingFrameCount-1));
        QPixmap combined(bot.size());
        QPainter p(&combined);
        QImage wm(":/images/wm.png");

        p.drawImage(QPoint(0, 0), bot); // drawn as-is
        p.setOpacity(ui->sliderStyleStrength->value()/100.0);
        p.drawImage(QPoint(0, 0), top);
        p.end();


        //Draw water mark
        QPainter p2(&combined);

        /*
          +---------------------------+
          |              5            |
          |<----5------>wm<----5----->|
          |              5            |
          +---------------------------+

           Rect width = wm.width+10
           Rect height =wm.height+10
           Rect x      =imgwidth-width
           Rect y      =img height-height
                                          */
        int pad=8;
        int rectWidth  = wm.width()+(pad*2);
        int rectHeight = wm.height()+(pad*2);
        int rectLeft    = bot.width()-rectWidth;
        int rectTop     = bot.height()-rectHeight;

        /*
        Water mark image
        Point x      =rectLeft+pad
        Point y      =rectTop+pad  */

        int wmLeft    = rectLeft+pad;
        int wmTop     = rectTop+pad;


        p2.fillRect(QRect(rectLeft,rectTop,rectWidth,rectHeight), QBrush(QColor(0, 0, 0, 128)));
        p2.drawImage(QPoint(wmLeft, wmTop), wm);
        p2.end();
        combined.save(paths->getStyledFramePath()+"ns"+lstFrames.at(processingFrameCount-1));

        // ui->labelStyleImage->setPixmap(QPixmap(paths->getStyledFramePath()+"ns"+lstFrames.at(processingFrameCount-1)).scaled(QSize(120,90),Qt::KeepAspectRatio,Qt::FastTransformation));
        // ui->labelStyleImage->setEnabled(true);
        emit updatePreviewFrame(paths->getStyledFramePath()+"ns"+lstFrames.at(processingFrameCount-1));
        qDebug()<<"Style Frame ok";
    }

    //delete fastNeuralStyle;
    fastNeuralStyle->deleteLater();
    ui->progressBarOverall->setValue(81);

    if (ui->pushButtonPause->text().compare("Pause")==0)
        chainerProcess();

}

void NeuralStylerWindow::chainerProcess()
{

    processingFrameCount++;
    if (processingFrameCount<=lstFrames.count()){

        ui->labelStatus->setText("Artistic styling frame "+QString::number(processingFrameCount)+" of "+QString::number(lstFrames.count()));
        float per=(float)processingFrameCount/(float)lstFrames.count();
        ui->progressBarFrame->setValue(per*100);

        fastNeuralStyle=new StyleChainer();
        fastNeuralStyle->setAppPath(paths->getAppPath());
        connect(fastNeuralStyle,SIGNAL(started()),this,SLOT(fastNeuralStyleStarted()));
        connect(fastNeuralStyle,SIGNAL(finished(int)),this,SLOT(fastNeuralStyleCompleted(int)));
        QStringList arguments;
        arguments << paths->getAppPath()+"env/generate.py" <<  paths->getFramePath()+lstFrames.at(processingFrameCount-1)
                  <<"-m"<<paths->getStylePath()+ui->comboBoxStyles->currentText()+".model"<<
                    "-o"<<paths->getStyledFramePath()+lstFrames.at(processingFrameCount-1);
        fastNeuralStyle->setArguments(arguments);
        fastNeuralStyle->sytleIt();
    } else {

        createStyledVideo();

    }
}
void NeuralStylerWindow::createStyledVideo()
{
    ui->pushButtonPause->setText("Pause");
    ui->pushButtonPause->setEnabled(false);
    ui->pushButtonViewFrame->setEnabled(false);

    //./ffmpeg -framerate 25 -i out/frames/frame%05d.jpg -i jp_audio.wav  -c:a aac -strict -2 -y gif/testgv.mp4

    ffmpeg=new FfmpegProcess();
    connect(ffmpeg,SIGNAL(started()),this,SLOT(styledVideoStarted()));
    connect(ffmpeg,SIGNAL(finished(int)),this,SLOT(styledVideoCompleted(int)));

    QStringList arguments;

    QFileInfo fileInfo(QFile(ui->lineEditFilePath->text()));
    //Handle Video ->MP4
    if(fileInfo.suffix().toLower().compare("gif")!=0) {

        if (QFile(paths->getAudioPath()+"audio.wav").exists()&& QFile(paths->getAudioPath()+"audio.wav").size()>0&&strFramesPerSec.compare("")!=0){

            //video
            arguments  << "-framerate" << strFramesPerSec
                       <<"-i"<<paths->getStyledFramePath()+"nsframe%05d.jpg"
                      <<"-i"<< paths->getAudioPath()+"audio.wav"
                     <<"-c:a"<<"aac"<< "-strict"<<"-2"
                    <<"-y"<<paths->getStyledVideoPath()+fileInfo.baseName()+"_"+ui->comboBoxStyles->currentText()+".mp4";

        } else {

            //Video with No audio
            QDir frameDir(paths->getStyledFramePath());
            QStringList filters;
            QStringList lstStyledFrames;
            filters<<"*.jpg";
            QFileInfo fileInfo(QFile(ui->lineEditFilePath->text()));

            frameDir.setNameFilters(filters);
            lstStyledFrames.clear();
            lstStyledFrames=frameDir.entryList();

            //Can be image
            if (lstStyledFrames.count()==2) { // Image


                // Just copy
                if (QFile::exists(paths->getStyledVideoPath()+fileInfo.baseName()+"_"+ui->comboBoxStyles->currentText()+".jpg"))
                    QFile::remove(paths->getStyledVideoPath()+fileInfo.baseName()+"_"+ui->comboBoxStyles->currentText()+".jpg");
                QFile(paths->getStyledFramePath()+"frame00001.jpg").copy(paths->getStyledVideoPath()+fileInfo.baseName()+"_"+ui->comboBoxStyles->currentText()+".jpg");

                styleIntensityDlg=new StyleIntensityDialog(this,paths->getFramePath()+lstFrames.at(0),
                                                           paths->getStyledFramePath()+lstFrames.at(0),
                                                           paths->getStyledVideoPath()+fileInfo.baseName()+"_"+ui->comboBoxStyles->currentText()+".jpg");
                styleIntensityDlg->show();

                finishStyling();
                ffmpeg->deleteLater();
                return ;

            } else {
                //Video with out audio
                arguments  << "-framerate" << strFramesPerSec
                           <<"-i"<<paths->getStyledFramePath()+"nsframe%05d.jpg"
                          <<"-y"<<paths->getStyledVideoPath()+fileInfo.baseName()+"_"+ui->comboBoxStyles->currentText()+".mp4";

            }


        }

    } else {

        //GIF
        arguments  << "-framerate" << strFramesPerSec
                   <<"-i"<<paths->getStyledFramePath()+"nsframe%05d.jpg"
                  <<"-y"<<paths->getStyledVideoPath()+fileInfo.baseName()+"_"+ui->comboBoxStyles->currentText()+".gif";

    }


    ffmpeg->setArguments(arguments);
    ffmpeg->startFfmpeg();


}

void NeuralStylerWindow::styledVideoStarted()
{
    qDebug()<<"creating styled video";
    ui->labelStatus->setText("Creating artwork...");

}
void NeuralStylerWindow::styledVideoCompleted(int ec)
{
    qDebug()<<"Styled video created";

    ffmpeg->deleteLater();
    finishStyling();
}

void NeuralStylerWindow::gotFrame(QString fnum)
{
    int64_t fnumer=fnum.toLongLong();

    if(frameCount>0&&fnum>0)
    {

        float per=(float)fnumer/(float)frameCount;
        qDebug()<<"frame extract:"<<QString::number(per*100);

        ui->progressBarFrame->setValue(per*100);
    }
}

void NeuralStylerWindow::on_comboBoxStyles_currentIndexChanged(const QString &arg1)
{
    qDebug()<<arg1;
    QPixmap pixmap(paths->getStylePath()+arg1+"-style.jpg");
    if (pixmap.height()>0){
        ui->labelStyleImage->setPixmap(pixmap);
    }
    else{
        ui->labelStyleImage->setText("No preview");
    }
}

void NeuralStylerWindow::on_pushButtonOpenOutput_clicked()
{

    QDesktopServices::openUrl(QUrl::fromLocalFile(paths->getStyledVideoPath()));
}

void NeuralStylerWindow::on_actionAbout_triggered()
{
    aboutDlg=new AboutDialog(this);
    aboutDlg->show();
}
void NeuralStylerWindow::update()
{
    //qDebug()<<tm.toString("hh:mm:ss");
    ui->labelElapsedTime->setText( QTime(0,0,0,0).addMSecs(tElapsed.elapsed()).toString("hh:mm:ss"));
}

void NeuralStylerWindow::finishStyling()
{
    ui->pushButtonProcess->setText("Create Artistic Style");
    ui->pushButtonProcess->setIcon(QPixmap(":/images/paint.png"));

    ui->labelStatus->setText("Styling completed.");
    ui->groupBox->setDisabled(false);
    ui->groupBox_2->setDisabled(false);
    timer->stop();
    ui->progressBarOverall->setValue(100);
    ui->progressBarFrame->setValue(100);
    if (lstFrames.count()>0){

        if (isImageOrGif()){
            if (isGif())
                QMessageBox::information(this,"NeuralStyler","Artistic gif created.");
            else
                QMessageBox::information(this,"NeuralStyler","Artistic image created.");
            ui->groupBox_2->setEnabled(false);
        }
        else{
            QMessageBox::information(this,"NeuralStyler","Artistic video created.");
        }
    }
    //delete ffmpeg;
}
void NeuralStylerWindow::seek(int pos)
{


    mp->command("pausing seek "+QString::number(pos)+" 2");
    // qDebug()<<QString::number(ui->frameSlider->lowerValue())+"="+QString::number(ui->frameSlider->upperValue());
    if (ui->frameSlider->lowerValue()<ui->frameSlider->upperValue()){

        QTime posStart(0,0,0);
        QTime posEnd(0,0,0);
        QTime styleDur(0,0,0);
        int styleDuration=ui->frameSlider->upperValue()-ui->frameSlider->lowerValue();
        videoTimeLabel->setText("Length : "+styleDur.addSecs(styleDuration).toString("hh:mm:ss"));
        QString strVideoStatus="Style video from "+posStart.addSecs(ui->frameSlider->lowerValue()).toString("hh:mm:ss")+" to "+posEnd.addSecs(ui->frameSlider->upperValue()).toString("hh:mm:ss")+"";
        videoStatusLabel->setText(strVideoStatus);
    }
    else{
        videoStatusLabel->setText("<font color=\"red\">Invalid Duration!</font>");
    }
}
void NeuralStylerWindow::setSliderRange(float dur)
{
    int duration=dur;
    ui->frameSlider->setRange(0, duration);
    ui->frameSlider->setLowerValue(0);
    ui->frameSlider->setUpperValue(duration);
    QTime tmDuration(0,0,0);
    QString strVideoDuration="Length :"+tmDuration.addSecs(duration).toString("hh:mm:ss");
    videoTimeLabel->setText(strVideoDuration);
    if(bitSettingsFirstRun->testBit(1)){
        ui->frameSlider->setUpperPosition(settings->value("State/trimup",1).toInt());
        ui->frameSlider->setLowerPosition(settings->value("State/trimlow",0).toInt());
        bitSettingsFirstRun->setBit(1,0);
    }

}
bool NeuralStylerWindow::isImageOrGif()
{
    QFileInfo fileInfo(QFile(ui->lineEditFilePath->text()));

    if(fileInfo.suffix().toLower().compare("jpeg")==0)
        return true;
    if(fileInfo.suffix().toLower().compare("jpg")==0)
        return true;
    if(fileInfo.suffix().toLower().compare("png")==0)
        return true;
    if(fileInfo.suffix().toLower().compare("gif")==0)
        return true;
    if(fileInfo.suffix().toLower().compare("bmp")==0)
        return true;
    if(fileInfo.suffix().toLower().compare("ppm")==0)
        return true;
    if(fileInfo.suffix().toLower().compare("pnm")==0)
        return true;
    if(fileInfo.suffix().toLower().compare("pgm")==0)
        return true;
    if(fileInfo.suffix().toLower().compare("pbm")==0)
        return true;
    if(fileInfo.suffix().toLower().compare("webp")==0)
        return true;
    if(fileInfo.suffix().toLower().compare("bpg")==0)
        return true;

    return false;
}
bool NeuralStylerWindow::isGif()
{
    QFileInfo fileInfo(QFile(ui->lineEditFilePath->text()));

    if(fileInfo.suffix().toLower().compare("gif")==0)
        return true;

    return false;
}

void NeuralStylerWindow::on_pushButtonPause_clicked()
{
    if (ui->pushButtonPause->text().compare("Pause")==0){
        ui->pushButtonPause->setText("Paused");
        timer->stop();
        ui->progressBarFrame->setEnabled(false);
        ui->progressBarOverall->setEnabled(false);
    }
    else{

        ui->pushButtonPause->setText("Pause");
        //resume
        chainerProcess();
        timer->start();
        ui->progressBarFrame->setEnabled(true);
        ui->progressBarOverall->setEnabled(true);
    }
}

void NeuralStylerWindow::on_checkBoxNoScale_clicked()
{
    if(ui->checkBoxNoScale->isChecked()){
        ui->comboBoxResolution->setDisabled(true);
        ui->checkBoxKeepAspectRatio->setDisabled(true);
        ui->lineEditHeight->setDisabled(true);
        ui->lineEditWidth->setDisabled(true);

    }
    else{
        ui->comboBoxResolution->setDisabled(false);
        ui->checkBoxKeepAspectRatio->setDisabled(false);
        ui->lineEditHeight->setDisabled(false);
        ui->lineEditWidth->setDisabled(false);
    }
}

void NeuralStylerWindow::on_pushButonBrowseFileSave_clicked()
{
    QString dir = paths->getDir(this,"Open a Directory for output:","");
    if (dir.isEmpty())
        return;

    ui->lineEditSavePath->setText(dir);
    settings->beginGroup("Main");
    settings->setValue("destination", dir);
    settings->endGroup();
    paths->setStyledVideoPath(ui->lineEditSavePath->text());
}

void NeuralStylerWindow::on_actionGet_more_styles_triggered()
{
    QDesktopServices::openUrl(QUrl("http://neuralstyler.com/styles.html"));
}

void NeuralStylerWindow::on_sliderStyleStrength_valueChanged(int value)
{
    ui->labelStyleS->setText("Style strength :"+QString::number(value)+"%");
}


void NeuralStylerWindow::on_comboBoxResolution_currentIndexChanged(const QString &arg1)
{
    if (arg1.compare("Custom")==0)
    {
        ui->groupBoxRes->setVisible(true);

    }
    else{
        ui->groupBoxRes->setVisible(false);
    }
}
void NeuralStylerWindow::engineState(bool isSave)
{
    if (isSave){
        settings->beginGroup("State");
        settings->setValue("processfile", ui->lineEditFilePath->text());
        settings->setValue("keepaspectratio", ui->checkBoxKeepAspectRatio->isChecked());
        settings->setValue("noscale", ui->checkBoxNoScale->isChecked());
        settings->setValue("resindex", ui->comboBoxResolution->currentIndex());
        settings->setValue("width", ui->lineEditWidth->text());
        settings->setValue("height", ui->lineEditHeight->text());
        settings->setValue("height", ui->lineEditHeight->text());
        settings->setValue("stylestrength", ui->sliderStyleStrength->value());
        settings->setValue("trimup", ui->frameSlider->upperValue());
        settings->setValue("trimlow", ui->frameSlider->lowerValue());
        settings->endGroup();

    }
    else{

        ui->lineEditFilePath->setText(settings->value("State/processfile","").toString());
        ui->checkBoxKeepAspectRatio->setChecked(settings->value("State/keepaspectratio",true).toBool());
        ui->checkBoxNoScale->setChecked(settings->value("State/noscale",false).toBool());
        ui->comboBoxResolution->setCurrentIndex(settings->value("State/resindex",0).toInt());
        ui->sliderStyleStrength->setValue(settings->value("State/stylestrength",50).toInt());
        if (ui->checkBoxNoScale->isChecked()){
            ui->checkBoxKeepAspectRatio->setEnabled(false);
            ui->comboBoxResolution->setEnabled(false);
            ui->lineEditHeight->setEnabled(false);
            ui->lineEditWidth->setEnabled(false);
        }


    }

}
void NeuralStylerWindow::initVideoTrim()
{
    ui->pushButtonPause->setEnabled(false);
    ui->pushButtonViewFrame->setEnabled(false);
    if (!isImageOrGif()){
        ui->groupBox_2->setEnabled(true);
        videoTimeLabel->setText("");
        if(mp){
            mp=new Mplayer();
            mp->setMplayerPath(mPath);
            connect(mp,SIGNAL(gotduration(float)),this,SLOT(setSliderRange(float)));
            connect(ui->frameSlider, SIGNAL(lowerValueChanged(int)), SLOT(seek(int)));
            connect(ui->frameSlider, SIGNAL(upperValueChanged(int)), SLOT(seek(int)));
            mp->setVideoWin(mplayerVideo->winId());
#ifdef Q_OS_LINUX
    mp->setScaling(261,181);
#endif


        }
        //m_mpv->command(QStringList() << "loadfile" << QUrl::fromLocalFile(ui->lineEditFilePath->text()).toString());
        mp->play( QUrl::fromLocalFile(ui->lineEditFilePath->text()).toString());
        // const bool paused = m_mpv->getProperty("pause").toBool();
        //if (!paused)
        //   m_mpv->setProperty("pause", !paused);
    }else{
        ui->groupBox_2->setEnabled(false);
        bitSettingsFirstRun->setBit(1,false);

    }
}
void NeuralStylerWindow::showEvent(QShowEvent * event)
{
    if (bitSettingsFirstRun->testBit(0)){
        initVideoTrim();
        bitSettingsFirstRun->setBit(0,false);
    }

}
void NeuralStylerWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    if (ui->pushButtonProcess->text().compare("Cancel Neural Styling")==0)
    {if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation?",
                                                   "You haven't finished styling yet. Do you want to exit without finishing?",
                                                   QMessageBox::Yes|QMessageBox::No))
        {
            if(ffmpeg){
                ffmpeg->write("q\n");
                ffmpeg->waitForBytesWritten();
            }
            // qDebug()<<"wrote q";
            event->accept();

        }
    }
    else
    {
        event->accept();


    }
    engineState(true);

}


void NeuralStylerWindow::on_action_Help_triggered()
{
    QDesktopServices::openUrl(QUrl("http://neuralstyler.com/help/"));
}

void NeuralStylerWindow::on_pushButtonReset_clicked()
{
    ui->comboBoxResolution->setCurrentIndex(0);
    ui->sliderStyleStrength->setValue(50);
    ui->checkBoxKeepAspectRatio->setChecked(true);
    ui->checkBoxNoScale->setChecked(false);
    ui->comboBoxResolution->setEnabled(true);
}

void NeuralStylerWindow::on_pushButtonViewFrame_clicked()
{

    frameDlg=new FrameDialog(this);
    QObject::connect(this,SIGNAL(updatePreviewFrame(QString)),frameDlg,SLOT(LoadFrame(QString)));
    frameDlg->show();
}
void NeuralStylerWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    if (mimeData->hasUrls())
    {
        QStringList pathList;
        QList<QUrl> urlList = mimeData->urls();

        // extract the local paths of the files
        for (int i = 0; i < urlList.size(); i++)
        {
            pathList.append(urlList.at(i).toLocalFile());
        }
        if(pathList.size()>0)
            ui->lineEditFilePath->setText(pathList.at(0));
        initVideoTrim();
    }
    //event->acceptProposedAction()
}
void NeuralStylerWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}
void NeuralStylerWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();

}
