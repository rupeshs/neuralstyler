/*
    NeuralStyler,Artistic style for your videos
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

#include "paths.h"

Paths::Paths(QString appPath)
{

    //All path init
    strAppPath=appPath+QDir::separator();
    strFramePath = appPath+QDir::separator()+"frames";
    strAudioPath = appPath+QDir::separator()+"audio";
    strStyledFramePath = appPath+QDir::separator()+"sframes";
    strStyledVideoPath = appPath+QDir::separator()+"styledvideo";
    strStylePath=appPath+QDir::separator()+"styles";

}

void Paths::ensureFramePath()
{
    ensurePath(strFramePath);

}
void Paths::ensureAudioPath()
{
    ensurePath(strAudioPath);

}
void Paths::ensureStyledFramePath()
{
    ensurePath( strStyledFramePath);

}
void Paths::ensureStyledVideoPath()
{
    ensurePath( strStyledVideoPath);

}
void Paths::ensurePath(QString path)
{

    if(!QDir(path).exists())
    {
        QDir(path).mkdir(path);
    }
}

