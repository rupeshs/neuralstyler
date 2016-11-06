#ifndef STYLEINTENSITYDIALOG_H
#define STYLEINTENSITYDIALOG_H

#include <QDialog>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QFile>
namespace Ui {
class StyleIntensityDialog;
}

class StyleIntensityDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StyleIntensityDialog(QWidget *parent = 0,QString srcPath="",QString stylePath="",QString destPath="");
    ~StyleIntensityDialog();
     void changeStyleStrength(int styleStrength);

private slots:
    void on_sliderStyleIntensity_valueChanged(int value);
    void on_pushButtonSave_clicked();

private:
    Ui::StyleIntensityDialog *ui;
    QString strStylePath;
    QString strSourcePath;
    QString strSavePath;

};

#endif // STYLEINTENSITYDIALOG_H
