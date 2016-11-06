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
paint icon :http://findicons.com/icon/85674/paint?id=85674

*/

#ifndef NEURALSTYLERWINDOW_H
#define NEURALSTYLERWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <ffmpegprocess.h>
#include <stylechainer.h>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <paths.h>
#include <QTime>
#include <QDesktopServices>
#include <QMessageBox>
#include <aboutdialog.h>
#include <QPointer>
#include <QTimer>
#include <QSettings>
#include <Qpainter>
#include <styleintensitydialog.h>
#include <QBitArray>
#include "qxtspanslider.h"
#include "mpvwidget.h"
#include <QCloseEvent>
#include "framedialog.h"
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>

namespace Ui {
class NeuralStylerWindow;
}

class NeuralStylerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NeuralStylerWindow(QWidget *parent = 0);
    ~NeuralStylerWindow();
    static int const EXIT_CODE_REBOOT = -123456789;

signals:
    void updatePreviewFrame(QString);
private slots:
    void on_pushButonBrowseFile_clicked();
    void on_pushButtonProcess_clicked();

    void fpsStarted();
    void fpsCompleted(int ec);

    void frameExtratorStarted();
    void frameExtratorCompleted(int ec);

    void audioExtratorStarted();
    void audioExtratorCompleted(int ec);

    void fastNeuralStyleStarted();
    void fastNeuralStyleCompleted(int ec);

    void styledVideoStarted();
    void styledVideoCompleted(int ec);

    void gotFrame(QString fnum);

    void on_comboBoxStyles_currentIndexChanged(const QString &arg1);

    void on_pushButtonOpenOutput_clicked();

    void on_actionAbout_triggered();
    void update();
    void finishStyling();

    void seek(int pos);
    void setSliderRange(int duration);
    bool isImageOrGif();
    bool isGif();


    void on_pushButtonPause_clicked();
    void on_checkBoxNoScale_clicked();

    void on_pushButonBrowseFileSave_clicked();

    void on_actionGet_more_styles_triggered();

    void on_sliderStyleStrength_valueChanged(int value);

    void on_comboBoxResolution_currentIndexChanged(const QString &arg1);
    void engineState(bool save);
    void initVideoTrim();

    void showEvent(QShowEvent * event);
    void closeEvent(QCloseEvent *event);


    void on_action_Help_triggered();

    void on_pushButtonReset_clicked();

    void on_pushButtonViewFrame_clicked();
    void dropEvent(QDropEvent* event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);

private:
    Ui::NeuralStylerWindow *ui;

    QPointer<FfmpegProcess> ffmpeg;
    QPointer<StyleChainer> fastNeuralStyle;

    Paths *paths;
    MpvWidget *m_mpv;

    QStringList lstFrames;

    void getFramePerSec();//Get fps using ffmpeg
    void extractFrames(); //Extract all frames
    void extractAudio(); //Extract Audio
    void styleFrames(); //Style frames
    void chainerProcess();//invoke fast neural style transfer
    void createStyledVideo();//Save video

    long processingFrameCount;

    QString strFramesPerSec;
    QString strDuration;
    long elaspsedTime;
    int64_t frameCount;

    AboutDialog  *aboutDlg;
    StyleIntensityDialog  *styleIntensityDlg;
    FrameDialog *frameDlg;
    QTime tElapsed;
    QTimer *timer;

    QLabel *videoTimeLabel;
    QLabel *videoStatusLabel;

    QSettings *settings;
    QBitArray *bitSettingsFirstRun;


};

#endif // NEURALSTYLERWINDOW_H
