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
#include "incredimail_2.h"

#include <QSqlDatabase>
#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>
#include <QFile>

Incredimail_2::Incredimail_2()
{
    sql_db = QSqlDatabase::addDatabase("QSQLITE");
}


bool Incredimail_2::Set_SQLite_File(QString Database) {

    if( QFile::exists(Database) ) {
       sql_db.setDatabaseName(Database);
       qDebug() << "Opening SQL Database :" << Database;
    } else {
       qDebug() << "Does not exist SQL Database :" << Database;
    }

    return sql_db.open();
}


void Incredimail_2::Close_SQLite_File() {
    sql_db.close();
    qDebug() << "Closing SQL Database";
}


void Incredimail_2::Get_Email_Offset_and_Size(int &file_offset, int &size, int email_index, int &deleted_email ) {
QString prepare;
QSqlQuery query;
QString ContainerId;

   if( sql_db.isOpen() && sql_db.isValid() && IM_Database.exists() ) {
       prepare.clear();
       // First, find the ContainerID in the sqlite database
       prepare = QString("SELECT containerID FROM CONTAINERS WHERE FILENAME='%1'").arg(IM_Database_Info.baseName());
       query.prepare(prepare);
       query.exec();
       query.first();
       ContainerId =  query.value(0).toString();

       // Set up for the main query, such as offset, size, and if it is deleted
       prepare.clear();
       prepare = QString("SELECT MsgPos,LightMsgSize,Deleted FROM Headers WHERE containerID='%1' ORDER BY MsgPos ASC").arg(ContainerId);
       query.exec(prepare);
       query.first();

       // Loop though the email index
       for(int i = 0; i < email_index; i++ ) {
          query.next();
       }

       file_offset   = query.value(0).toInt();
       size          = query.value(1).toInt();
       deleted_email = query.value(2).toInt();
       query.finish();
   } else {
       qDebug() << "Problem with the (Containers) SQLite Database";
       qDebug() << " SQL Open       : " << sql_db.isOpen();
       qDebug() << " SQL Valid      : " << sql_db.isValid();
       qDebug() << " Database Exists: " << IM_Database.exists();
   }
}


void Incredimail_2::Email_Count(int &email_total, int &total_deleted ) {
QString prepare;
QSqlQuery query;
QString ContainerId;
int deleted = 0;

    if( sql_db.isOpen() && sql_db.isValid() && IM_Database.exists() ) {
        // Setup for Mailbox
        prepare = QString("SELECT msgscount,containerID FROM CONTAINERS WHERE FILENAME='%1'").arg(IM_Database_Info.baseName());
        query.prepare(prepare);
        qDebug() << "1 Email Count() - Prepare QString: " << prepare;
        query.exec();
        query.first();
        email_total = query.value(0).toInt();
        ContainerId = query.value(1).toString();

        // Set up for the deleted emails
        prepare.clear();
        prepare = QString("SELECT Deleted FROM Headers WHERE containerID='%1'").arg(ContainerId);
        query.exec(prepare);
        qDebug() << "2 Email Count() - Prepare QString: " << prepare;
        query.first();
        deleted = query.value(0).toInt();

        while(query.next()) {
           deleted += query.value(0).toInt();
        }

        total_deleted = deleted;
        email_total  += deleted;
        query.finish();
    }
}
