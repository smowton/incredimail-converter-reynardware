#include "incredimail_convert.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFile>
#include <QDebug>
#include <QSqlRecord>
#include <QVariant>

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


void Incredimail_Convert::Insert_Attachments( QString eml_filename, QString attachments_path, QString final_email_filename ) {

/*
HANDLE inputfile, outputfile, encoded_file;
HANDLE encode64_input_file, encode64_output_file;

DWORD byteswritten;
char string_1[512], string_2[512];
char attachment_name[512];
int attachment_length, read_length, read_encoded_length;

char temp_path[MAX_CHAR];
char temp_filename[MAX_CHAR];


   inputfile  = CreateFile(eml_filename, GENERIC_READ, 0x0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
   outputfile = CreateFile(final_email_filename, GENERIC_WRITE, 0x0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
   read_length = 1;

   GetTempPath( sizeof( temp_path ), temp_path );

   if( inputfile && outputfile ) {
      while( read_length != 0 ) {
         memset( string_1, 0, MAX_CHAR );
         read_length = ReadOneLine( inputfile, string_1, MAX_CHAR );

         // search for the ATTACHMENT string
         if( strncmp( ATTACHMENT,  string_1, 34 ) == 0 ) {
            // fix the attachment string
            attachment_length = (int) strlen(string_1);
            strcpy( attachment_name, attachments_path );
            strcat( attachment_name, "\\" );
            strncat( attachment_name, &string_1[34], attachment_length - 36 );

            // encode the attachement
            GetTempFileName( temp_path, "att", 0, temp_filename );
            encode64_input_file  = CreateFile(attachment_name, GENERIC_READ, 0x0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
            encode64_output_file = CreateFile(temp_filename, GENERIC_WRITE, 0x0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY, NULL );
            if( encode64_input_file && encode64_output_file  ) {
               encode( encode64_input_file, encode64_output_file, 72 );
               CloseHandle( encode64_input_file );
               CloseHandle( encode64_output_file );

               encoded_file = CreateFile(temp_filename, GENERIC_READ, 0x0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
               if( encoded_file ) {
                  read_encoded_length = 1;
                  while( read_encoded_length ) {
                     memset( string_2, 0, MAX_CHAR );
                     read_encoded_length = ReadOneLine( encoded_file, string_2, MAX_CHAR );
                     if( outputfile ) {
                        WriteFile( outputfile, string_2, read_encoded_length, &byteswritten, NULL );
                     }
                  }
                  CloseHandle( encoded_file );
                  DeleteFile( temp_filename );
               }
            }
         } else {
            WriteFile( outputfile, string_1, read_length, &byteswritten, NULL );
         }
      }
   }
   CloseHandle( inputfile );
   CloseHandle( outputfile );
   */
}
