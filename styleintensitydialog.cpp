#include "styleintensitydialog.h"
#include "ui_styleintensitydialog.h"

StyleIntensityDialog::StyleIntensityDialog(QWidget *parent,QString srcPath,QString stylePath,QString destPath) :
    QDialog(parent),
    ui(new Ui::StyleIntensityDialog)
{
    ui->setupUi(this);
    // ui->sliderStyleIntensity->setValue(100);
    strSourcePath=srcPath;
    strStylePath=stylePath;
    strSavePath=destPath;
    QImage top(strStylePath);
    QImage bot(strSourcePath);
    // both images are opaque because JPEG has no alpha channel
    QPixmap combined(bot.size());
    QPainter p(&combined);
    p.drawImage(QPoint(0, 0), bot); // drawn as-is
    p.setOpacity(1.0);
    p.drawImage(QPoint(0, 0), top);
    p.end();
    ui->labelCanvas->setPixmap(combined);

    changeStyleStrength(50);
    ui->sliderStyleIntensity->setValue(50);
    if (QFile::exists(strSavePath))
        QFile::remove(strSavePath);
    ui->labelCanvas->pixmap()->save(strSavePath);


}

StyleIntensityDialog::~StyleIntensityDialog()
{
    delete ui;
}

void StyleIntensityDialog::on_sliderStyleIntensity_valueChanged(int value)
{
    ui->label->setText("Style strength :"+QString::number(value)+"%");
    changeStyleStrength(value);
}


void StyleIntensityDialog::on_pushButtonSave_clicked()
{
    if (QFile::exists(strSavePath))
        QFile::remove(strSavePath);
    ui->labelCanvas->pixmap()->save(strSavePath);


}
void StyleIntensityDialog::changeStyleStrength(int styleStrength)
{
    QImage top(strStylePath);
    QImage bot(strSourcePath);
    QImage wm(":/images/wm.png");

    QPixmap combined(bot.size());
    QPainter p(&combined);

    p.drawImage(QPoint(0, 0), bot); // drawn as-is
    p.setOpacity(styleStrength/100.0);
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

    ui->labelCanvas->setPixmap(combined);
}
