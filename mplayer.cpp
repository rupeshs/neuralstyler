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

#include "mplayer.h"

Mplayer::Mplayer()
{


    mProcess=new MyProcess();
    QObject::connect(mProcess,SIGNAL(lineAvailable(QByteArray)),this,SLOT(mplayerConsole(QByteArray)));
    // QObject::connect(mProcess,SIGNAL(finished(int)),this,SLOT(emitProcessFinished(int)));

}

//Life of MPlayer
void Mplayer::mplayerConsole(QByteArray ba)
{
//qDebug()<<ba;
QString mplayerOutputLine(ba);
//Stream duration
if(mplayerOutputLine.contains("ID_LENGTH",Qt::CaseInsensitive)){


    tmpstr=parsevalue("ID_LENGTH=","=",mplayerOutputLine);
    _duration=tmpstr.toFloat();
    emit gotduration(_duration);
    _tduration=  _tduration.addSecs(_duration);
    qDebug()<<QString::number(_duration);


}
}
void Mplayer::setVideoWin(long int id)
{
    argfrontEnd<<"-wid"<<QString::number(id);

}

void Mplayer::play(QString filepath)
{
    mProcess->clearArguments();

        #ifdef Q_OS_WIN
            argfrontEnd<<"-slave"<<"-identify"<<"-noquiet"<<"-priority"<<"abovenormal";
            argfrontEnd<<"-vo"<<"direct3d,directx";


#endif
#if defined(Q_OS_LINUX) || defined(Q_OS_OPENBSD)
    argfrontEnd<<"-slave"<<"-identify"<<"-noquiet";
#endif
     argfrontEnd<<"-volume"<<"0";

    argfrontEnd<<filepath;

    QString strArgument;

    mProcess->addArgument(mplayerPath);


    foreach (strArgument, argfrontEnd) {
        mProcess->addArgument(strArgument);
    }

   // qDebug()<< mProcess->arguments();

    mProcess->start();

     mProcess->write("osd 0\n");
     mProcess->write("pause\n");


}

QString Mplayer::parsevalue( QString serstr,QString sep,QString str)
{
    QString tmp;
    tmp=str.mid(str.indexOf(serstr));
    tmp=tmp.left(tmp.indexOf("\n"));

    qDebug()<<"Parsed ->"<<tmp.mid(tmp.indexOf(sep)+1);

    return tmp.mid(tmp.indexOf(sep)+1).trimmed();
}
void  Mplayer::command(QString cmd)
{
    mProcess->write(cmd.toLatin1()+"\n");
    //qDebug()<<cmd;

}
