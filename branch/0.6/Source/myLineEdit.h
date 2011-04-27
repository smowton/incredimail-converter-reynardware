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
#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QLineEdit>
class QDropEvent;

class myLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    myLineEdit(QWidget *parent = 0);

protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
};

#endif // MYLINEEDIT_H
