#include "incredimail_convert.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFile>
#include <QDebug>
#include <QSqlRecord>
#include <QVariant>
#include <QDir>

Incredimail_Convert::Incredimail_Convert()
{
// Todo!
}

void Incredimail_Convert::Set_Database_Path(QString Path) {
    IM_Database_Info.setFile(Path);
}


bool Incredimail_Convert::Set_Database_File(QString Database) {

    bool ret = false;

    IM_Database.setFileName(Database);
    if( IM_Database.exists() ) {
        ret = IM_Database.open(QIODevice::ReadOnly);
    }

    return ret;
}


void Incredimail_Convert::Close_Database_File() {
    IM_Database.close();
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


void Incredimail_Convert::Insert_Attachments( QString eml_filename ) {

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
               // still need the attachment
               attachment_name = IM_Attachment.path();
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


bool Incredimail_Convert::Set_Attachments_Directory( QString attachment_path ) {

    bool set_succussfully = false;

    IM_Attachment.setFile(attachment_path);

    if(IM_Attachment.isDir()) {
        set_succussfully = true;
    }

    return set_succussfully;
}


QStringList Incredimail_Convert::Setup_IM_Directory_Processing( ) {

    QStringList filters, file_listing;
    QDir tmp_dir;

    filters << "*.imm";

    tmp_dir.setNameFilters(filters);
    tmp_dir.setPath(IM_Database_Info.path());
    file_listing = tmp_dir.entryList(QDir::NoDotAndDotDot | QDir::Files );

    return file_listing;
}
