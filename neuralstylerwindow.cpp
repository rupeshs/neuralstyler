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
    // createStyledVideo();
}

NeuralStylerWindow::~NeuralStylerWindow()
{
    delete ui;
}

void NeuralStylerWindow::on_pushButonBrowseFile_clicked()
{

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open video"), "", tr("videoFiles (*.*)"));
    ui->lineEditFilePath->setText(fileName);
    //QFileInfo fileInfo(QFile(ui->lineEditFilePath->text()));
    //qDebug()<<   fileInfo.baseName();
}

void NeuralStylerWindow::on_pushButtonProcess_clicked()
{
    tElapsed.start();
    timer->start(1000);

    if (!QFile(ui->lineEditFilePath->text()).exists()){
        QMessageBox::critical(this,qApp->applicationName(),"Please provide a valid file!");
        return;
    }
    //ui->pushButtonProcess->setText("Cancel");
    ui->progressBarOverall->setValue(0);
    ui->groupBox->setDisabled(true);

    //Clean folders
    QDir(paths->getAudioPath()).removeRecursively();
    QDir(paths->getFramePath()).removeRecursively();
    QDir(paths->getStyledFramePath()).removeRecursively();

    paths->ensureFramePath();
    paths->ensureAudioPath();
    paths->ensureStyledFramePath();
    paths->ensureStyledVideoPath();

    getFramePerSec();

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
    ui->progressBarOverall->setValue(33);
    //./ffmpeg -i strip_jp.mkv -vf scale=320:240 frames/frame%05d.jpg
    ffmpeg=new FfmpegProcess();
    connect(ffmpeg,SIGNAL(started()),this,SLOT(frameExtratorStarted()));
    connect(ffmpeg,SIGNAL(finished(int)),this,SLOT(frameExtratorCompleted(int)));
    connect(ffmpeg,SIGNAL(gotFrame(QString)),this,SLOT(gotFrame(QString)));
    QStringList arguments;
    //arguments << "-i" << ui->lineEditFilePath->text()<<"-vf"<<"scale="+ui->comboBoxResolution->currentText().replace("x",":")<<"frames/frame%05d.jpg";
    arguments << "-i" << ui->lineEditFilePath->text()<<"-vf"<<"scale="+ui->comboBoxResolution->currentText().replace("x",":")<<paths->getFramePath()+"frame%05d.jpg";

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

    arguments << "-i" << ui->lineEditFilePath->text()<<"-y"<<paths->getAudioPath()+"audio.wav";
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
    styleFrames();

}

void NeuralStylerWindow::styleFrames()
{
    QDir framesDir(paths->getFramePath());
    QStringList filters;
    filters<<"*.jpg";
    framesDir.setNameFilters(filters);
    lstFrames.clear();
    lstFrames=framesDir.entryList();
    qDebug()<<lstFrames;

    //python generate.py rup.jpeg -m models/seurat.model -o rupstyle2.jpg
    processingFrameCount=0;
    chainerProcess();

}
void NeuralStylerWindow::fastNeuralStyleStarted()
{
    qDebug()<<"Style start";

}
void NeuralStylerWindow::fastNeuralStyleCompleted(int ec)
{
    qDebug()<<"Style Frame ok";

    //delete fastNeuralStyle;
    fastNeuralStyle->deleteLater();
    ui->progressBarOverall->setValue(81);

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

    //./ffmpeg -framerate 25 -i out/frames/frame%05d.jpg -i jp_audio.wav  -c:a aac -strict -2 -y gif/testgv.mp4

    ffmpeg=new FfmpegProcess();
    connect(ffmpeg,SIGNAL(started()),this,SLOT(styledVideoStarted()));
    connect(ffmpeg,SIGNAL(finished(int)),this,SLOT(styledVideoCompleted(int)));

    QStringList arguments;
    QFileInfo fileInfo(QFile(ui->lineEditFilePath->text()));

    //Handle Video ->MP4
    if(fileInfo.suffix().toLower().compare("gif")!=0) {

        if (QFile(paths->getAudioPath()+"audio.wav").exists()&&strFramesPerSec.compare("")!=0){

            //video
            arguments  << "-framerate" << strFramesPerSec
                       <<"-i"<<paths->getStyledFramePath()+"frame%05d.jpg"
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
            if (lstStyledFrames.count()==1) { // Image

                // Just copy
                if (QFile::exists(paths->getStyledVideoPath()+fileInfo.baseName()+"_"+ui->comboBoxStyles->currentText()+".jpg"))
                    QFile::remove(paths->getStyledVideoPath()+fileInfo.baseName()+"_"+ui->comboBoxStyles->currentText()+".jpg");
                QFile(paths->getStyledFramePath()+"frame00001.jpg").copy(paths->getStyledVideoPath()+fileInfo.baseName()+"_"+ui->comboBoxStyles->currentText()+".jpg");
                finishStyling();
                ffmpeg->deleteLater();
                return ;

            } else {
                //Video with out audio
                arguments  << "-framerate" << strFramesPerSec
                           <<"-i"<<paths->getStyledFramePath()+"frame%05d.jpg"
                          <<"-y"<<paths->getStyledVideoPath()+fileInfo.baseName()+"_"+ui->comboBoxStyles->currentText()+".mp4";

            }


        }

    } else {

        //GIF
        arguments  << "-framerate" << strFramesPerSec
                   <<"-i"<<paths->getStyledFramePath()+"frame%05d.jpg"
                  <<"-y"<<paths->getStyledVideoPath()+fileInfo.baseName()+"_"+ui->comboBoxStyles->currentText()+".gif";

    }


    ffmpeg->setArguments(arguments);
    ffmpeg->startFfmpeg();


}

void NeuralStylerWindow::styledVideoStarted()
{
    qDebug()<<"creating styled video";
    ui->labelStatus->setText("Creating artistic video...");

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
    else
    {
        ui->labelStyleImage->setText("No preview");
    }
}

void NeuralStylerWindow::on_pushButtonOpenOutput_clicked()
{

    QDesktopServices::openUrl(paths->getStyledVideoPath());
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
    ui->labelStatus->setText("Styled video created");
    ui->groupBox->setDisabled(false);
    timer->stop();
    ui->progressBarOverall->setValue(100);
    ui->progressBarFrame->setValue(100);
    QMessageBox::information(this,"NeuralStyler","Artistic video/gif created.");
    //delete ffmpeg;
}
