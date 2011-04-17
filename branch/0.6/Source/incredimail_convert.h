#ifndef INCREDIMAIL_CONVERT_H
#define INCREDIMAIL_CONVERT_H

#include <QString>
#include <QFile>
#include <QSqlDatabase>
#include <QFileInfo>

class Incredimail_Convert
{
public:
    Incredimail_Convert();
    bool Set_Database_File(QString Database);
    void Close_Database_File();
    bool Set_SQLite_File(QString Database);
    void Close_SQLite_File();
    void Incredimail_2_Email_Count(int &email_total, int &total_deleted);
    void Incredimail_2_Get_Email_Offset_and_Size(int &file_offset, int &size, int email_index, int &deleted_email );
    void Extract_EML_File(QString eml_filename, int offset, int size);
    void Incredimail_Convert::Insert_Attachments( QString eml_filename, QString attachments_path, QString final_email_filename );

private:
    QFile IM_Database;
    QSqlDatabase sql_db;
    QFileInfo IM_Database_Info;

};

#endif // INCREDIMAIL_CONVERT_H
