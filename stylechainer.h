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
#ifndef STYLECHAINER_H
#define STYLECHAINER_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QRegExp>

static QRegExp rx_time("(\\d*\\.\\d+)*.sec");

class StyleChainer: public QProcess
{
    Q_OBJECT

public:
    StyleChainer();
    void setAppPath(QString appPath){strAppPath=appPath;}
    QString getProcessingTime(){return strElaspedTime;}


public slots:
    void readyReadStandardOutput();
    void readyReadStandardError();
    void sytleIt();

private:
    QString strElaspedTime;
    QString strConsoleOut;
    QString strConsoleErr;
    QString strAppPath;
};

#endif // STYLECHAINER_H
