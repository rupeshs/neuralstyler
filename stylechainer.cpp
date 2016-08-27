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

#include "stylechainer.h"

StyleChainer::StyleChainer()
{
   connect(this,SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()));
}
void StyleChainer::readyReadStandardOutput()
{

    strConsoleOut+=readAllStandardOutput();
       qDebug()<< strConsoleOut;
    if (rx_time.indexIn(strConsoleOut)>-1) {

        qDebug()<<rx_time.cap(1);
        strElaspedTime=rx_time.cap(1);
    }



}
void StyleChainer::sytleIt()
{qDebug()<<arguments();
#ifdef Q_OS_WIN
     this->start(strAppPath+"env/python.exe",arguments());
#endif
#ifdef Q_OS_LINUX
     this->start(strAppPath+"env/bin/python",arguments());
# endif

}

