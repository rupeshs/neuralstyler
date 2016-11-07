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

#ifndef MPLAYER_H
#define MPLAYER_H
#include <QtCore>
#include <QtGui>
#include <QObject>
#include <myprocess.h>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QDebug>
#include <QTime>
class Mplayer:public QObject
{
    Q_OBJECT

public:

    Mplayer();

signals:
    void gotduration(float dur);
public slots:
    void mplayerConsole(QByteArray ba);
    void setMplayerPath(QString path){mplayerPath=path;}
    void setVideoWin(long id);
    void setScaling(int width,int height);
    void play(QString filepath);
    QString parsevalue( QString serstr,QString sep,QString str);
    void  command(QString cmd);
private:
    QString mplayerPath;
    float _duration;
    QString tmpstr;
    MyProcess *mProcess;
    QStringList argfrontEnd;
    QTime _tduration;

};

#endif // MPLAYER_H
