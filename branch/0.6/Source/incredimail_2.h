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
    //**************************************************************************
    // Input  : QString - The name of the SQLITE db
    //                    This QString can be the full path to the database
    // Output : bool
    //            - FALSE = failed to open the database
    //            - TRUE  = success to open the database
    // Description: This function sets the SQLITE database, that contains all the
    //              eml offset information
    //**************************************************************************

    void Close_SQLite_File();
    //**************************************************************************
    // Input  : none
    // Output : none
    // Description: This function closes the SQLITE database, which was set in the
    //              Set_SQLite_File() public function
    //**************************************************************************

    void Email_Count(int &email_total, int &total_deleted);
    //**************************************************************************
    // Input  : none
    // Output : integer email - total of email files which is marked not deleted
    //             in the database
    //          integer total_deleted - total of email file which is marked
    //             deleted in the database file
    // Description: This function sets the email_total and total_deleted varibles
    //              from a given IM Database (*.imm), which was set in the
    //              Set_Database_File() function in the base class.
    //**************************************************************************

    void Get_Email_Offset_and_Size(int &file_offset, int &size, int email_index, int &deleted_email );
    //**************************************************************************
    // Input  : interger email_index - the email index in the given IM Database
    //             (*.imm)
    // Output : integer file_offset - the file offset of the eml file in the given
    //             IM Database
    //          integer size - the file size of the eml file in the given IM
    //             Database
    //          integer deleted_email - this is a flag that indicates whether the
    //             eml file was marked for deletion
    // Description: This function retreives the file offset, size, and, whether
    //              it was marked for deletion based off of the .imm database
    //              file which was set Set_Database_File() member in the base class.
    //**************************************************************************

private:
    QSqlDatabase sql_db;  // this is the incredimail 2 database, usally named containers.db

};

#endif // INCREDIMAIL_2_H
