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
//     The Initial Developer of the Original Code is David P. Owczarski, created March 20, 2009
//
//     Contributor(s): Christopher Smowton <chris@smowton.net>
//
//************************************************************************************************

#ifndef __INCREDIMAIL_CONVERT__
#define __INCREDIMAIL_CONVERT__

#define S_LEN 1024*2
#define MAX_CHAR 256

#ifdef __cplusplus
extern "C" {
#endif

	enum INCREDIMAIL_VERSIONS {
		INCREDIMAIL_VERSION_UNKNOWN,
		INCREDIMAIL_XE,
		INCREDIMAIL_2,
		INCREDIMAIL_2_MAILDIR // Some versions of IM2 use a maildir-like format instead of a .imm file (~mbox).
							  // This might depend on precise version of IM, Windows version or both.
	};

	extern HWND global_hwnd;

	void email_count(const char *, int *, int *);
	//***************************************************************************
	// INPUTS:
	//
	// OUTPUTS:
	//
	// RETURN VALUE:
	//
	// DESCRIPTION:
	//
	//***************************************************************************

	void get_email_offset_and_size(char *, unsigned int *, unsigned int *, int, int, int *);
	//***************************************************************************
	// INPUTS:
	//
	// OUTPUTS:
	//
	// RETURN VALUE:
	//
	// DESCRIPTION:
	//
	//***************************************************************************

	void get_database_version(char *, char *);
	//***************************************************************************
	// INPUTS:
	//
	// OUTPUTS:
	//
	// RETURN VALUE:
	//
	// DESCRIPTION:
	//
	//***************************************************************************

	void extract_eml_files(const char *filename_data, char *, int offset, unsigned int size);
	//***************************************************************************
	// INPUTS:
	//
	// OUTPUTS:
	//
	// RETURN VALUE:
	//
	// DESCRIPTION:
	//
	//***************************************************************************

	void im_to_eml(char *eml_filename, const char *attachments_path, const char *);
	//***************************************************************************
	// INPUTS:
	//
	// OUTPUTS:
	//
	// RETURN VALUE:
	//
	// DESCRIPTION:
	//
	//***************************************************************************

	int DeleteDirectory(const char *);
	//***************************************************************************
	// INPUTS:
	//
	// OUTPUTS:
	//
	// RETURN VALUE:
	//
	// DESCRIPTION:
	//
	//***************************************************************************

	int ReadOneLine(HANDLE infile, char *buffer, int max_line_length);
	//***************************************************************************
	// INPUTS:
	//
	// OUTPUTS:
	//
	// RETURN VALUE:
	//
	// DESCRIPTION:
	//
	//***************************************************************************

	int FindDatabaseFiles(char *directory_search, char *temp_file_listing);
	//***************************************************************************
	// INPUTS:
	//
	// OUTPUTS:
	//
	// RETURN VALUE:
	//
	// DESCRIPTION:
	//
	//***************************************************************************

	enum INCREDIMAIL_VERSIONS FindIncredimailVersion(const char *file_or_directory);
	//***************************************************************************
	// INPUTS:
	//
	// OUTPUTS:
	//
	// RETURN VALUE:
	//
	// DESCRIPTION:
	//
	//***************************************************************************

	void Incredimail_2_Email_Count(const char *filename, int *email_total, int *deleted_emails);
	//***************************************************************************
	// INPUTS:
	//
	// OUTPUTS:
	//
	// RETURN VALUE:
	//
	// DESCRIPTION:
	//
	//***************************************************************************

	void Incredimail_2_Maildir_Email_Count(const char *filename, int *email_total, int *deleted_emails);

	void Incredimail_2_Get_Email_Offset_and_Size(const char *filename, unsigned int *file_offset, unsigned int *size, int email_index, int *deleted_email);
	//***************************************************************************
	// INPUTS:
	//
	// OUTPUTS:
	//
	// RETURN VALUE:
	//
	// DESCRIPTION:
	//
	//***************************************************************************

#ifdef __cplusplus // End extern "C"
}
#endif

#endif