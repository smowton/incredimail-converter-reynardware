#ifndef INCREDIMAIL_CONVERT_H
#define INCREDIMAIL_CONVERT_H

#include <QString>
#include <QFile>
#include <QFileInfo>

class Incredimail_Convert
{
public:
    Incredimail_Convert();
    bool Set_Database_File(QString Database);
    void Close_Database_File();
    void Extract_EML_File(QString eml_filename, int offset, int size);
    void Insert_Attachments( QString eml_filename, QString attachments_path );

protected:
    QFile IM_Database;
    QFileInfo IM_Database_Info;

};

#endif // INCREDIMAIL_CONVERT_H
