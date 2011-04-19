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
//          Created March 20, 2009
//          Updated April 19, 2011
//
//     Contributor(s):
//
//************************************************************************************************
#ifndef INCREDIMAIL_2_H
#define INCREDIMAIL_2_H

#include <incredimail_convert.h>

#include <QString>
#include <QSqlDatabase>

class Incredimail_2 : public Incredimail_Convert
{
public:
    Incredimail_2();

    bool Set_SQLite_File(QString Database);
    void Close_SQLite_File();
    void Email_Count(int &email_total, int &total_deleted);
    void Get_Email_Offset_and_Size(int &file_offset, int &size, int email_index, int &deleted_email );

private:
    QSqlDatabase sql_db;

};

#endif // INCREDIMAIL_2_H
