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
#include "ffmpegprocess.h"

FfmpegProcess::FfmpegProcess()
{

    connect(this,SIGNAL(readyReadStandardError()),this,SLOT(readyReadStandardError()));

}
void FfmpegProcess::startFfmpeg()
{
    qDebug()<<arguments();
    strFps="";

#ifdef Q_OS_WIN
    this->start("ffmpeg.exe",arguments());
#endif
#ifdef Q_OS_LINUX
    this->start("./ffmpeg",arguments());
# endif

}
void FfmpegProcess::readyReadStandardError()
{

    strConsoleErr+=readAllStandardError();
    QStringList strConsoleLines=strConsoleErr.split("\r");

    for (int i = 0; i < strConsoleLines.length();i++) {

        if (rx_frame_ct.indexIn( strConsoleLines.at(i))>-1) {

            // qDebug()<<rx_frame_ct.cap(1);
            QString frameCount=rx_frame_ct.cap(1);
            emit gotFrame(frameCount);

        }
    }
    if (rx_fps.indexIn(strConsoleErr)>-1) {

        // qDebug()<<rx_fps.cap(1);
        strFps=rx_fps.cap(1);
    }
    if (rx_fps_num.indexIn(strConsoleErr)>-1) {

        // qDebug()<<rx_fps_num.cap(1);
        if (strFps.compare("")==0)
            strFps=rx_fps_num.cap(1);
    }
    if (rx_duration.indexIn(strConsoleErr)>-1) {
        strDuration=rx_duration.cap(1);
        //qDebug()<<rx_duration.cap(1);
        // strFps=rx_time.cap(1);
    }
    //qDebug()<< strConsoleErr;

}
