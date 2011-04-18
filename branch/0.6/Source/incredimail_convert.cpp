#include "incredimail_convert.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFile>
#include <QDebug>
#include <QSqlRecord>
#include <QVariant>
#include <QTemporaryFile>

Incredimail_Convert::Incredimail_Convert()
{
// Todo!
}

bool Incredimail_Convert::Set_Database_File(QString Database) {

    bool ret = false;

    IM_Database.setFileName(Database);
    IM_Database_Info.setFile(Database);
    if( IM_Database.exists() ) {
        ret = IM_Database.open(QIODevice::ReadOnly);
    }

    return ret;
}


void Incredimail_Convert::Close_Database_File() {
    IM_Database.close();
}


bool Incredimail_Convert::Set_SQLite_File(QString Database) {

    sql_db = QSqlDatabase::addDatabase("QSQLITE");
    sql_db.setDatabaseName(Database);

    return sql_db.open();
}


void Incredimail_Convert::Close_SQLite_File() {
    sql_db.close();
}


void Incredimail_Convert::Incredimail_2_Get_Email_Offset_and_Size(int &file_offset, int &size, int email_index, int &deleted_email ) {
QString prepare;
QSqlQuery query;
QString ContainerId;

   if( sql_db.isOpen() && sql_db.isValid() && IM_Database.exists() ) {
       prepare.clear();
       prepare = QString("SELECT containerID FROM CONTAINERS WHERE FILENAME='%1'").arg(IM_Database_Info.baseName());
       query.prepare(prepare);
       query.exec();
       query.first();
       ContainerId =  query.value(0).toString();

       // Set up
       prepare.clear();
       prepare = QString("SELECT MsgPos,LightMsgSize,Deleted FROM Headers WHERE containerID='%1' ORDER BY MsgPos ASC").arg(ContainerId);
       query.exec(prepare);
       query.first();

       // Loop though the index
       for(int i = 0; i < email_index; i++ ) {
          query.next();
       }

       file_offset   = query.value(0).toInt();
       size          = query.value(1).toInt();
       deleted_email = query.value(2).toInt();
       query.finish();
   }
}


void Incredimail_Convert::Incredimail_2_Email_Count(int &email_total, int &total_deleted ) {
QString prepare;
QSqlQuery query;
QString ContainerId;
int deleted = 0;

    if( sql_db.isOpen() && sql_db.isValid() && IM_Database.exists() ) {
        // Setup for Mailbox
        prepare = QString("SELECT msgscount,containerID FROM CONTAINERS WHERE FILENAME='%1'").arg(IM_Database_Info.baseName());
        query.prepare(prepare);
        query.exec();
        query.first();
        email_total = query.value(0).toInt();
        ContainerId = query.value(1).toString();

        // Set up for the deleted emails
        prepare.clear();
        prepare = QString("SELECT Deleted FROM Headers WHERE containerID='%1'").arg(ContainerId);
        query.exec(prepare);
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


void Incredimail_Convert::Extract_EML_File(QString eml_filename, int offset, int size) {
QFile eml_file;
QByteArray extract;
int chunks = 0;
const int kbytes = 1024;

    // set the extraction eml file
    eml_file.setFileName( eml_filename );
    eml_file.open(QIODevice::WriteOnly);

    // seek the position in the imm file
    IM_Database.seek(offset);

    // set the chunks to be increaments of 1k
    chunks = size / kbytes;
    extract.resize(kbytes);

    for(int i = 0; i < chunks; i++ ) {
       IM_Database.read(extract.data(), kbytes);
       eml_file.write(extract.data(), kbytes);
    }

    chunks = size % kbytes;  // the rest of the data
    IM_Database.read(extract.data(), chunks);
    eml_file.write(extract.data(), chunks);
    eml_file.close();
}


void Incredimail_Convert::Insert_Attachments( QString eml_filename, QString attachments_path ) {

    QByteArray ATTACHMENT("----------[%ImFilePath%]----------");
    QString attachment_name, temp_eml_filename;
    QFile old_eml_file, temp_eml_file, attach;
    QByteArray extract, extract64;

    old_eml_file.setFileName(eml_filename);
    temp_eml_filename = eml_filename;
    temp_eml_file.setFileName(temp_eml_filename.append(".tmp"));
    temp_eml_file.open(QIODevice::WriteOnly);

    if( old_eml_file.exists() && temp_eml_file.isOpen() ) {
        old_eml_file.open(QIODevice::ReadOnly);

        while( !old_eml_file.atEnd() ) {
           extract = old_eml_file.readLine();
           // if the eml contain an attachment then base64 encode it
           if(extract.contains(ATTACHMENT)) {
               // still need the attachment path (maybe this needs to be a private var)
               attachment_name = attachments_path;
               attachment_name.append("/").append(extract.right(extract.size()-ATTACHMENT.size()));
               attachment_name = attachment_name.simplified();  // remove the junk

               // then base64
               attach.setFileName(attachment_name);
               if( attach.exists() ) {
                  attach.open(QIODevice::ReadOnly);
                  while(!attach.atEnd()) {
                     // why 54?  I am not 100% sure, but...this converts to 72 chars per line
                     extract64 = attach.read(54);
                     temp_eml_file.write(extract64.toBase64().append("\n"));
                  }
                  attach.close();
              } else {
                 qDebug() << "Error in attachment name:" << attachment_name;
              }
           } else {
              temp_eml_file.write(extract);
           }
        }
    }
    old_eml_file.remove();               // remove then close
    old_eml_file.close();
    temp_eml_file.rename(eml_filename);  // rename the temp to the final one
    temp_eml_file.close();               // then close it out
}
