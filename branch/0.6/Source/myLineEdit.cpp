#include <QtGui>

#include "myLineEdit.h"

myLineEdit::myLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setAcceptDrops(true);
}

void myLineEdit::dropEvent(QDropEvent *event)
{
    QString fName;
    QFileInfo info;
    QList<QUrl>urlList;

    if (event->mimeData()->hasUrls()) {
       urlList = event->mimeData()->urls(); // returns list of QUrls

       // if just text was dropped, urlList is empty (size == 0)
       if ( urlList.size() > 0) // if at least one QUrl is present in list
       {
          fName = urlList[0].toLocalFile(); // convert first QUrl to local path
          info.setFile( fName ); // information about file
          if ( info.isFile() ) {
             setText( fName );  // if is file, setText
          }
       }
    }
    event->acceptProposedAction();
}

void myLineEdit::dragEnterEvent(QDragEnterEvent *event)
{
        event->acceptProposedAction();
}
