//***********************************************************************************************
//     The contents of this file are subject to the Mozilla Public License
//     Version 1.1 (the "License"); you may not use this file except in
//     compliance with the License. You may obtain a copy of the License at
//     http://www.mozilla.org/MPL/
//
//     Software distributed under the License is distributed on an "AS IS"
//     basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
//     License for the specific language governing rights and limitations
//     under the License.
//
//     The Original Code is ReynardWare Incredimail Converter.
//
//     The Initial Developer of the Original Code is David P. Owczarski
//          Created March 20, 2009 (Versions 0.1, 0.2, 0.3, 0.53)
//          Updated April 27, 2011 (Versions 0.6)
//
//     Contributor(s):
//
//************************************************************************************************
#include <QtGui/QApplication>
#include <QFile>
#include <QTime>
#include <QTextStream>

#include "dialog.h"

void myMessageOutput(QtMsgType type, const char *msg) {
QFile file_handle;
QTime time, convert;
QString message;

    file_handle.setFileName("Reynardware Log.txt");
    file_handle.open(QIODevice::Append | QIODevice::Text);
    time = convert.currentTime();
    message  = time.toString("hh.mm.ss.zzz");
    message.append(" ");

    QTextStream out(&file_handle);
    switch (type) {
    case QtDebugMsg:
        message.append(QString("Debug   : %1").arg(msg));
        break;
    case QtWarningMsg:
        message.append(QString("Warning : %1").arg(msg));
        break;
    case QtCriticalMsg:
        message.append(QString("Critical: %1").arg(msg));
        break;
    case QtFatalMsg:
        message.append(QString("Fatal   : %1").arg(msg));
        abort();
    }
    out << message << endl;
    file_handle.close();
}


int main(int argc, char *argv[])
{
    qInstallMsgHandler(myMessageOutput);
    QApplication a(argc, argv);
    Dialog w;
    w.show();

    return a.exec();
}
