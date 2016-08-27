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

#ifndef PATHS_H
#define PATHS_H
#include<QString>
#include <QDir>
#include <QDebug>

class Paths
{
public:
    Paths(QString appPath);

    inline  QString getStyledVideoPath(){return strStyledVideoPath+"/";}
    inline  QString getAudioPath(){return strAudioPath+"/";}
    inline  QString getStylePath(){return strStylePath+"/";}
    inline  QString getFramePath(){return strFramePath+"/";}
    inline  QString getAppPath(){return strAppPath;}
    inline  QString getStyledFramePath(){return strStyledFramePath+"/";}

    void ensureFramePath();
    void ensureAudioPath();
    void ensureStyledFramePath();
    void ensureStyledVideoPath();

private:
    QString strAppPath;
    QString strFramePath;
    QString strAudioPath;
    QString strStyledFramePath;
    QString strStyledVideoPath;
    QString strStylePath;
    void ensurePath(QString pth);
};
typedef __int64 int64_t;

#endif // PATHS_H
