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
        message.append(QString("Debug   : <%1>").arg(msg));
        break;
    case QtWarningMsg:
        message.append(QString("Warning : <%1>").arg(msg));
        break;
    case QtCriticalMsg:
        message.append(QString("Critical: <%1>").arg(msg));
        break;
    case QtFatalMsg:
        message.append(QString("Fatal   : <%1>").arg(msg));
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
