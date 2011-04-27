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
