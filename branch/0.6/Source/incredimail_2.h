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
