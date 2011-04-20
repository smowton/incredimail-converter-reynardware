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
#ifndef INCREDIMAIL_CONVERT_H
#define INCREDIMAIL_CONVERT_H

#include <QString>
#include <QFile>
#include <QFileInfo>

class Incredimail_Convert
{
public:
    Incredimail_Convert();

    void Set_Database_Path(QString Path);
    //**************************************************************************
    // Input  : QString - Path of the Incredimail Database files
    // Output : None
    // Description: This function sets the protected variable IM_Database_Info
    //              to be used in other members.
    //**************************************************************************

    bool Set_Database_File(QString Database);
    //**************************************************************************
    // Input  : QString - The name of the *.imm database to extract the eml files
    //                   This QString can be the full path to the database
    // Output : bool
    //            - FALSE = failed to open the database
    //            - TRUE  = success to open the database
    // Description: This function sets the .imm database, which the eml files can
    //              be extracted from
    //**************************************************************************

    void Close_Database_File();
    //**************************************************************************
    // Input  : Takes no parameters
    // Output : Returns nothing
    // Description: This function closes the *.imm database, which is set in the
    //              Set_Database_File() function.
    //**************************************************************************

    void Extract_EML_File(QString eml_filename, int offset, int size);
    //**************************************************************************
    // Input  : QString eml_Filename - The eml file that will be created by offset
    //          and size.
    //        : int offset - The file offset or position in the imm database
    //        : int size   - The eml file size in the imm database
    // Output : None
    // Description: This function creates an eml file, named on the input QString
    //              eml_filename.  This routine extracts data by the file offset
    //              and size within the imm file.  The offset and size is
    //              retrived from the Incredimail2 or (TBD)IncredimailXE classes.
    //              The imm file is set by the Set_Database_File() routine, within
    //              this class.
    //**************************************************************************

    void Insert_Attachments( QString eml_filename );
    //**************************************************************************
    // Input  : QString eml_filename - the eml filename (including path) to have the
    //          the attachments inserted in.
    // Output : None
    // Description: This function given a eml file name and attachement path will
    //              insert the attachment if it finds the incredimail "attachement"
    //              string.   Then it will encode the attachment to base 64 (72 chars)
    //              in the eml file.
    //**************************************************************************

    bool Set_Attachments_Directory( QString attachment_path );
    //**************************************************************************
    // Input  : QString attachment_path - the attachments path, where the IM keeps
    //          the Incredimail attachments
    // Output : bool
    //              - true  = the attachment_path does exist
    //              - false = the attachment_path does not exist
    // Description: This function sets the attachment directory for the members of
    //              this class.
    //**************************************************************************

    QStringList Setup_IM_Directory_Processing( );
    //**************************************************************************
    // Input  : None
    // Output : QStringList - returns a listing of all the .imm files in the
    //          IM_Database_Info variable
    // Description: This function returns a listing of all the .imm files based
    //              on the protected variable IM_Database_Info
    //**************************************************************************

protected:
    QFile IM_Database;          // The imm database file handle
    QFileInfo IM_Database_Info; // The imm database directory
    QFileInfo IM_Attachment;    // The attachment directory
};

#endif // INCREDIMAIL_CONVERT_H
