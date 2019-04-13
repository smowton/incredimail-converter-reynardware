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
#define _CRT_SECURE_NO_WARNINGS  // remove the new mircosoft security warnings

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "sqlite3.h"
#include "increadimail_convert.h"
#include "base64.h"

const char *ATTACHMENT = "----------[%ImFilePath%]----------";

void email_count( const char *filename, int *email_total, int *deleted_emails ) {
int dummy = 1;
int i;
int e_count = 0;
int d_count = 0;
HANDLE helping_hand;
char version[5];
unsigned int size = 0;
unsigned int file_size;

   ZeroMemory( &version, sizeof( version ) );
   helping_hand = CreateFile( filename, GENERIC_READ, 0x0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
   
   if( helping_hand ) {
      ReadFile( helping_hand, &version, 0x04, &dummy, NULL );
   }

   SetFilePointer( helping_hand, 0, NULL, FILE_BEGIN );
   do {
      SetFilePointer( helping_hand, 0x0C, NULL, FILE_CURRENT );  // header
      ReadFile( helping_hand, &size, 0x01, &dummy, NULL );
      if( size == 0x02 ) {
        d_count++;
      }
      SetFilePointer( helping_hand, 0x19, NULL, FILE_CURRENT );  // header

      for( i = 0; i < 3; i++ ) {
         ReadFile( helping_hand, &size, 0x04, &dummy, NULL );
         if( !strncmp( version, "V#05", 4 ) ) {
            SetFilePointer( helping_hand, size * 2, NULL, FILE_CURRENT );
         } else {
            SetFilePointer( helping_hand, size, NULL, FILE_CURRENT );
         }
      }

      SetFilePointer( helping_hand, 0x0C, NULL, FILE_CURRENT );
      ReadFile( helping_hand, &size, 0x04, &dummy, NULL );
      if( !strncmp( version, "V#05", 4 ) ) {
         SetFilePointer( helping_hand, size * 2, NULL, FILE_CURRENT );
      } else {
         SetFilePointer( helping_hand, size, NULL, FILE_CURRENT );
      }

      SetFilePointer( helping_hand, 0x06, NULL, FILE_CURRENT );
      ReadFile( helping_hand, &file_size, 0x04, &dummy, NULL );
      SetFilePointer( helping_hand, 0x08, NULL, FILE_CURRENT );
      e_count++;
   } while( dummy );
   CloseHandle( helping_hand );
   e_count--;

   *deleted_emails = d_count;
   *email_total   = e_count;
}


void get_email_offset_and_size( char *filename, unsigned int *file_offset, unsigned int *size, int email_index, int e_count, int *deleted_email ) {
HANDLE helping_hand;
int dummy = 1;
int i, j;
char version[5];
unsigned int file_size, sizer;

   ZeroMemory( &version, sizeof( char ) * 5 );

   helping_hand = CreateFile( filename, GENERIC_READ, 0x00000000, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
   ReadFile( helping_hand, &version, 0x04, &dummy, NULL );
   
   SetFilePointer( helping_hand, 0, NULL, FILE_BEGIN );
   for( i = 0; i < email_index + 1; i++ ) {
      sizer = 0;
      SetFilePointer( helping_hand, 0x0C, NULL, FILE_CURRENT );  // header
      ReadFile( helping_hand, &sizer, 0x01, &dummy, NULL );
      if( sizer == 0x02 ) {
         *deleted_email = 1;
      } else {
         *deleted_email = 0;
      }
      SetFilePointer( helping_hand, 0x19, NULL, FILE_CURRENT );

      for( j = 0; j < 3; j++ ) {
         ReadFile( helping_hand, &sizer, 0x04, &dummy, NULL );
         if( !strncmp( version, "V#05", 4 ) ) {
            SetFilePointer( helping_hand, sizer * 2, NULL, FILE_CURRENT );
         } else {
            SetFilePointer( helping_hand, sizer, NULL, FILE_CURRENT );
         }
      }

      SetFilePointer( helping_hand, 0x0C, NULL, FILE_CURRENT );      
      ReadFile( helping_hand, &sizer, 0x04, &dummy, NULL );
      if( !strncmp( version, "V#05", 4 ) ) {
         SetFilePointer( helping_hand, sizer * 2, NULL, FILE_CURRENT );
      } else {
         SetFilePointer( helping_hand, sizer, NULL, FILE_CURRENT );
      }
      SetFilePointer( helping_hand, 0x06, NULL, FILE_CURRENT );
      file_size = 0;
      ReadFile( helping_hand, &file_size, 0x04, &dummy, NULL );
      *size = file_size;

      // I think this is the file offset
      ReadFile( helping_hand, &sizer, 0x04, &dummy, NULL );
      *file_offset = sizer;
      
      SetFilePointer( helping_hand, 0x04, NULL, FILE_CURRENT );
   }
   CloseHandle( helping_hand );
}


void extract_eml_files( const char *filename_data, char *eml_filename, int offset, unsigned int size ) {
HANDLE helping_hand;
HANDLE writing_hand;
int j, k, dummy;
char extract_data[1024];

      helping_hand = CreateFile( filename_data, GENERIC_READ, 0x0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
      writing_hand = CreateFile( eml_filename, GENERIC_WRITE, 0x0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
      SetFilePointer( helping_hand, offset, NULL, FILE_BEGIN );
      k = size / 1024;
      for( j = 0; j < k; j++ ) {
         ReadFile( helping_hand, &extract_data, 1024, &dummy, NULL );
         WriteFile( writing_hand, &extract_data, 1024, &dummy, NULL );
      }
      k = size % 1024;
      ReadFile( helping_hand, &extract_data, k, &dummy, NULL );
      WriteFile( writing_hand, &extract_data, k, &dummy, NULL );
      CloseHandle( helping_hand );
      CloseHandle( writing_hand );
}

void insert_attachments( char *im_filename, const char *attachments_path, const char *final_email_filename ) {

HANDLE inputfile, outputfile, encoded_file;
HANDLE encode64_input_file, encode64_output_file;

DWORD byteswritten;
char string_1[512], string_2[512];
char attachment_name[512];
int attachment_length, read_length, read_encoded_length;

char temp_path[MAX_CHAR];
char temp_filename[MAX_CHAR];


   inputfile  = CreateFile(im_filename, GENERIC_READ, 0x0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
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
}

enum find_multipart_states {
	FMS_START,
	FMS_CONTENTTYPE,
	FMS_HEADEREND1,
	FMS_HEADEREND2,
	FMS_BOUNDARY,
	FMS_DONE
};

void im_to_eml(char* im_filename, const char* attachments_path, const char* eml_filename) {

	// Step 1: modify in-place if necessary to correct a Mime content-type that uses imbndry instead of boundary.
	char buf[4096];
	char first_boundary[256];
	first_boundary[0] = '\0';

	FILE* fin = fopen(im_filename, "r");
	enum find_multipart_states state = FMS_START;

	while (fgets(buf, 4096, fin) && first_boundary[0] == '\0') {
		int linelen = strlen(buf);
		if (linelen > 0 && buf[linelen - 1] != '\n')
			fprintf(stderr, "May have missed an important header in %s\r\n", im_filename);
		switch (state) {
		case FMS_START:
			if (strstr(buf, "Content-Type: multipart"))
				state = FMS_CONTENTTYPE;
			// Fall through
		case FMS_CONTENTTYPE:
			if (strstr(buf, "imbndary="))
				state = FMS_HEADEREND1;
			break;
		case FMS_HEADEREND1:
			if (!strcmp(buf, "\n"))
				state = FMS_HEADEREND2;
			break;
		case FMS_HEADEREND2:
			if (!strcmp(buf, "\n"))
				state = FMS_BOUNDARY;
			break;
		case FMS_BOUNDARY:
			if (!strncmp(buf, "--", 2)) {
				const char* lineend = strchr(buf, '\n');
				int copychars = strlen(buf) - 2;
				if (!lineend)
					fprintf(stderr, "MIME boundary without newline?");
				else
					--copychars;
				strncpy_s(first_boundary, 256, buf + 2, copychars);
				state = FMS_DONE;
			}
			break;
		default:
			break;
		}
	}

	if (first_boundary[0] != '\0') {

		int needed = strlen(im_filename) + 10;
		char* fixed_name = malloc(needed);
		if (!fixed_name) {
			fprintf(stderr, "Failed to allocate %d bytes in im_to_eml\n", needed);
			exit(1);
		}
		
		sprintf_s(fixed_name, needed, "%s_fixed", im_filename);

		fseek(fin, 0, SEEK_SET);
		FILE* fout = fopen(fixed_name, "w");
		if (!fout) {
			fprintf(stderr, "Failed to open %s in im_to_eml\n", fixed_name);
			exit(1);
		}

		state = FMS_START;

		while (fgets(buf, 4096, fin)) {
			
			switch (state) {
			case FMS_START:
				if (strstr(buf, "Content-Type: multipart"))
					state = FMS_CONTENTTYPE;
				// Fall through
			case FMS_CONTENTTYPE:
			{
				char* boundary_loc = strstr(buf, "imbndary=");
				if(boundary_loc) {
					int off = boundary_loc - buf;
					sprintf_s(boundary_loc, 4096 - off, "boundary=\"%s\"\n", first_boundary);
					state = FMS_DONE;
				}
				break;
			}
			default:
				break;
			}

			fwrite(buf, 1, strlen(buf), fout);

		}

		fclose(fout);
		fclose(fin);

		DeleteFile(im_filename);
		MoveFile(fixed_name, im_filename);

	}
	else {

		fclose(fin);

	}

	// Step 2: Weave in any attachments that IncrediMail has placed out-of-line:
	insert_attachments(im_filename, attachments_path, eml_filename);

}

void get_database_version(char *database, char *version ) {
HANDLE helping_hand;
int dummy = 1;
char temp_version[5];

   ZeroMemory( &temp_version, sizeof( temp_version ) );
   helping_hand = CreateFile( database, GENERIC_READ, 0x00000000, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
   ReadFile( helping_hand, &temp_version, 0x04, &dummy, NULL );
   CloseHandle( helping_hand );

   strcpy( version, temp_version );

}

int DeleteDirectory(const char *sPath) {
  SHFILEOPSTRUCT fileop;
  int ret;

  ZeroMemory( &fileop, sizeof( SHFILEOPSTRUCT ) );

  fileop.hwnd                  = NULL;
  fileop.wFunc                 = FO_DELETE;
  fileop.pTo                   = NULL;
  fileop.fFlags                = FOF_NOCONFIRMATION|FOF_SILENT;  
  fileop.fAnyOperationsAborted = FALSE;
  fileop.lpszProgressTitle     = NULL;
  fileop.hNameMappings         = NULL;

  fileop.pFrom = sPath;

  ret = SHFileOperation(&fileop);

  return( ret );
}

int ReadOneLine( HANDLE infile, char *buffer, int max_line_length ) {
DWORD byteread;
char byte;   
int index, end = 0;

   // initialize variables
   byteread = 1;
   end = 0;
   index = 0;

   // loop will end if:
   //    (1) there are no more bytes to read in the file
   //    (2) a DOS/Windows line return is found
   //    (3) max line length in the buffer
   while( byteread != 0 && !end && index < max_line_length ) {
      ReadFile( infile, &byte, 0x01, &byteread, NULL );
      if( byteread != 0 ) {
         buffer[index] = byte;
         index++;
         // test if it is line feed
         if( byte == 0x0D ) {
            ReadFile( infile, &byte, 0x01, &byteread, NULL );
            if( byte == 0x0A && byteread != 0 ) {
               buffer[index] = byte;
               index++;
               end = 1;
            }
         }
      }
   }

   return index;
}

int FindDatabaseFiles( char *directory_search, char *temp_file_listing ) {
WIN32_FIND_DATA FindFileData;
HANDLE hFind, list_output;

char database_filename[MAX_CHAR];
char temp_path[MAX_CHAR];
char temp_filename[MAX_CHAR];
char temp_string[MAX_CHAR];

DWORD result_create_temp, dummy;

int total_count = 1;

   ZeroMemory( &database_filename, sizeof( database_filename ) );
   ZeroMemory( &temp_path, sizeof( temp_path ) );
   ZeroMemory( &temp_filename, sizeof( temp_filename ) );
   ZeroMemory( &temp_string, sizeof( temp_string ) );

   strcpy( database_filename, directory_search );
   strcat( database_filename, "\\*.imm" );

   // Get temp windows path
   result_create_temp = GetTempPath( sizeof( temp_path ), temp_path );
   GetTempFileName( temp_path, "lst", 0, temp_filename );
   
   list_output = CreateFile(temp_filename, GENERIC_WRITE, 0x0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
   
   hFind = FindFirstFile(database_filename, &FindFileData);
   if (hFind == INVALID_HANDLE_VALUE)
	   return -1;

   sprintf(temp_string, "%s\\%s%c%c", directory_search, FindFileData.cFileName, 0x0D, 0x0A );
   WriteFile( list_output, temp_string, (DWORD) strlen( temp_string ), &dummy, NULL );

   while( FindNextFile(hFind, &FindFileData) != 0 ) {
      sprintf(temp_string, "%s\\%s%c%c", directory_search, FindFileData.cFileName, 0x0D, 0x0A );
      WriteFile( list_output, temp_string, (DWORD) strlen( temp_string ), &dummy, NULL );
      total_count++;
   }

   strcpy( temp_file_listing, temp_filename );

   FindClose( hFind );
   CloseHandle( list_output );

   return total_count;
}

static int testimdb(const char* filename) {

	struct _stat statbuf;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	int ret = 0;

	if (_stat(filename, &statbuf) != 0)
		return 0;

	if (sqlite3_open_v2(filename, &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK)
		return 0;

	if (sqlite3_prepare_v2(db, "select count(*) from Containers", -1, &stmt, NULL) != SQLITE_OK)
		goto outdb;

	if (sqlite3_step(stmt) != SQLITE_ROW)
		goto outstmt;

	if (sqlite3_column_int(stmt, 0) == 0)
		goto outstmt;

	ret = 1;

outstmt:
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

outdb:
	sqlite3_close(db);

	return ret;

}

static int endswith_case_insensitive(const char *s, const char *suffix) {
	size_t suffix_len = strlen(suffix);
	size_t s_len = strlen(s);

	return s_len >= suffix_len && !stricmp(s + (s_len - suffix_len), suffix);
}

enum INCREDIMAIL_VERSIONS FindIncredimailVersion(const char *file_or_directory) {
	char temp_path[MAX_CHAR];
	enum INCREDIMAIL_VERSIONS ret = INCREDIMAIL_VERSION_UNKNOWN;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	size_t file_or_directory_len = strlen(file_or_directory);
	const char *file_or_directory_end = file_or_directory + file_or_directory_len;

	// First check if a specific file was given:
	const char *xe_suffix = ".imh";
	const char *im2_mbox_suffix = "\\containers.db";
	const char *im2_maildir_suffix = "\\messageStore.db";
	
	if (endswith_case_insensitive(file_or_directory, xe_suffix))
		return INCREDIMAIL_XE;
	else if (endswith_case_insensitive(file_or_directory, im2_mbox_suffix) && testimdb(file_or_directory))
		return INCREDIMAIL_2;
	else if (endswith_case_insensitive(file_or_directory, im2_maildir_suffix) && testimdb(file_or_directory))
		return INCREDIMAIL_2_MAILDIR;

	// Otherwise try searching a directory:

	// is there an Incredimail XE header file 
	strcpy(temp_path, file_or_directory);
	strcat(temp_path, "\\*.imh");
	hFind = FindFirstFile(temp_path, &FindFileData);
	FindClose(hFind);

	const char *found_files[3];
	size_t n_tests_succeeded = 0;

	if (hFind != INVALID_HANDLE_VALUE) {
		ret = INCREDIMAIL_XE;
		found_files[n_tests_succeeded++] = "* At least one .imh file, as used by Incredimail XE\n";
	}

	// is there an Incredimail 2 database, i.e. containers.db 
	strcpy(temp_path, file_or_directory);
	strcat(temp_path, "\\containers.db");

	if (testimdb(temp_path)) {
		ret = INCREDIMAIL_2;
		found_files[n_tests_succeeded++] = "* A containers.db file, as used by earlier versions of Incredimail 2\n";
	}

	// Is there a messageStore.db, indicating maildir format?
	strcpy(temp_path, file_or_directory);
	strcat(temp_path, "\\messageStore.db");
	if (testimdb(temp_path)) {
		ret = INCREDIMAIL_2_MAILDIR;
		found_files[n_tests_succeeded++] = "* A MessageStore.db file, as used by newer versions of Incredimail 2\n";
	}

	if (n_tests_succeeded > 1) {
		const char *msgHeader = "Multiple Incredimail database types were found:\n";
		const char *msgFooter = "The last one (as the newest format) has been selected for now, but you might want to try selecting an individual database to convert";
		size_t msgLength = strlen(msgHeader) + strlen(msgFooter);
		for (size_t i = 0; i < n_tests_succeeded; ++i) {
			msgLength += strlen(found_files[i]);
		}
		msgLength++; // For final null terminator

		char *msg = (char *)malloc(msgLength);
		msg[0] = '\0';
		if (!msg) {
			MessageBox(global_hwnd, msg, "Failed to allocate memory in FindIncredimailVersion", MB_OK);
			exit(1);
		}

		strcat_s(msg, msgLength, msgHeader);
		for (size_t i = 0; i < n_tests_succeeded; ++i) {
			strcat_s(msg, msgLength, found_files[i]);
		}
		strcat_s(msg, msgLength, msgFooter);

		MessageBox(global_hwnd, msg, "Warning", MB_OK);

		free(msg);
	}

    return ret;
}

void Incredimail_2_Maildir_Email_Count(const char *filename, int *email_total, int *deleted_emails) {

	sqlite3 *db;
	sqlite3_stmt *stmt;

	const char* allMailQuery = "select count(*) from Headers where Location != \"\"";
	const char* deletedQuery = "select count(*) from Headers where Location != \"\" and deleted = 1";

	*email_total = 0;
	*deleted_emails = 0;

	if (sqlite3_open(filename, &db) != SQLITE_OK) {
		char msg[4096];
		snprintf(msg, 4096, "Unable to open sqlite3 DB %s", filename);
		MessageBox(global_hwnd, msg, "Error!", MB_OK);
		return;
	}

	if (sqlite3_prepare_v2(db, allMailQuery, -1, &stmt, NULL) != SQLITE_OK)
		goto outdb;

	if (sqlite3_step(stmt) != SQLITE_ROW)
		goto outstmt;

	*email_total = sqlite3_column_int(stmt, 0);

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	if (sqlite3_prepare_v2(db, deletedQuery, -1, &stmt, NULL) != SQLITE_OK)
		goto outdb;

	if (sqlite3_step(stmt) != SQLITE_ROW)
		goto outstmt;

	*deleted_emails = sqlite3_column_int(stmt, 0);

outstmt:

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

outdb:

	sqlite3_close(db);

}

void Incredimail_2_Email_Count( const char *filename, int *email_total, int *deleted_emails ) {
char sql[MAX_CHAR];
char trimmed_filename[MAX_CHAR];
char containerID[MAX_CHAR];
char temp_dir[MAX_CHAR];
char container_path[MAX_CHAR];

const char *tail;
char *pdest;
int rc, del, deleted;
sqlite3 *db;
sqlite3_stmt *stmt;

   deleted = 0;

   memset( temp_dir, 0, sizeof(temp_dir) );
   memset( container_path, 0, sizeof(container_path) );
   strcpy( temp_dir, filename );
   pdest = strrchr( temp_dir, '\\' );
   strncpy( container_path, temp_dir, strlen( temp_dir ) - strlen( pdest ) );
   strcat( container_path, "\\Containers.db" );

   rc = sqlite3_open(container_path, &db);

   if( rc ) {
     // no printf....
     printf("can't open db\n");
   } else {
      // Zero out strings
      memset( sql, 0, sizeof(sql) );
      memset( trimmed_filename, 0, sizeof(trimmed_filename) );
      memset( containerID, 0, sizeof( containerID ) );

      // The filename minus the '.imm'
      strncpy( trimmed_filename, &pdest[1], strlen(pdest) );
      pdest = strrchr( trimmed_filename, '.' );
      trimmed_filename[strlen(trimmed_filename) - strlen(pdest)] = '\0';
   
      sprintf(sql, "SELECT msgscount,containerID FROM CONTAINERS WHERE FILENAME='%s'", trimmed_filename);

      rc = sqlite3_prepare_v2( db, sql, (int) strlen( sql ), &stmt, &tail );

      // debug***************
      if( rc == SQLITE_OK ) {
         printf("OK!\n");
      }
      //*********************

      rc = sqlite3_step( stmt );

      // only get the first column and result
      *email_total = sqlite3_column_int(stmt,0);
      strcpy(containerID,sqlite3_column_text(stmt,1));

      // reset the sql statement
      sqlite3_reset( stmt );
      sqlite3_finalize( stmt );
      sqlite3_close( db );

      // setup for deleted emails
      rc = sqlite3_open(container_path, &db);
      memset( &sql, 0, sizeof(sql) );
      sprintf(sql, "SELECT Deleted FROM Headers WHERE containerID='%s'", containerID);
      rc = sqlite3_prepare_v2( db, sql, (int) strlen( sql ), &stmt, &tail );

      // debug***************
      if( rc == SQLITE_OK ) {
         printf("OK!\n");
      }
      //*********************
      rc = sqlite3_step( stmt );

      while( rc == SQLITE_ROW ) {
         del = sqlite3_column_int(stmt,0);
         printf("%s\n",sqlite3_column_text(stmt,0));
         if( del == 1 ) {
            deleted++;
         }
         rc = sqlite3_step( stmt );
      }
      sqlite3_reset( stmt );
      sqlite3_finalize( stmt );
   }

   // Incredimail 2 email total doesn't count delete emails
   *deleted_emails = deleted;
   *email_total += deleted;
   sqlite3_close( db );
}


void Incredimail_2_Get_Email_Offset_and_Size( const char *filename, unsigned int *file_offset, unsigned int *size, int email_index, int *deleted_email ) {

sqlite3 *db;
sqlite3_stmt *stmt;
char sql[MAX_CHAR];
char trimmed_filename[MAX_CHAR];
char containerID[MAX_CHAR];
char temp_dir[MAX_CHAR];
char container_path[MAX_CHAR];
const char *tail;
int i, rc;
char *pdest;

   memset( temp_dir, 0, sizeof(temp_dir) );
   memset( container_path, 0, sizeof(container_path) );
   strcpy( temp_dir, filename );
   pdest = strrchr( temp_dir, '\\' );
   strncpy( container_path, temp_dir, strlen( temp_dir ) - strlen( pdest ) );
   strcat( container_path, "\\Containers.db" );

   rc = sqlite3_open(container_path, &db);

   if( rc ) {
     // no printf....
     // printf("can't open db\n");
   } else {   
      // Zero out strings
      memset( sql, 0, sizeof(sql) );
      memset( trimmed_filename, 0, sizeof(trimmed_filename) );
      memset( containerID, 0, sizeof( containerID ) );

      // The filename minus the '.imm'
      strncpy( trimmed_filename, &pdest[1], strlen(pdest) );
      pdest = strrchr( trimmed_filename, '.' );
      trimmed_filename[strlen(trimmed_filename) - strlen(pdest)] = '\0';

      sprintf(sql, "SELECT containerID FROM CONTAINERS WHERE FILENAME='%s'", trimmed_filename);

      rc = sqlite3_prepare_v2( db, sql, (int) strlen( sql ), &stmt, &tail );

      // debug***************
      if( rc == SQLITE_OK ) {
          printf("OK!\n");
      }
      //*********************

      rc = sqlite3_step( stmt );

      // only get the first column and result
      strcpy(containerID,sqlite3_column_text(stmt,0));

      // reset the sql statement
      sqlite3_reset( stmt );
      sqlite3_finalize( stmt );
      sqlite3_close( db );   

      // setup next query
      rc = sqlite3_open(container_path, &db);
      memset( &sql, 0, sizeof(sql) );
      sprintf(sql, "SELECT MsgPos,LightMsgSize,Deleted FROM Headers WHERE containerID='%s' ORDER BY MsgPos ASC", containerID);
      rc = sqlite3_prepare_v2( db, sql, (int) strlen( sql ), &stmt, &tail );


      // debug***************
      //if( rc == SQLITE_OK ) {
      //   printf("OK!\n");
      //}
      //*********************
      rc = sqlite3_step( stmt );
      email_index--;  // the index has to be decremented for the correct index in the sqlite db

      // Loop though the index
      for(i = 0; i <= email_index; i++ ) {
         rc = sqlite3_step( stmt );
      }

      // I love sqlite conversions
      *file_offset   = sqlite3_column_int(stmt,0);
      *size          = sqlite3_column_int(stmt,1);
      *deleted_email = sqlite3_column_int(stmt,2);

      sqlite3_reset( stmt );
      sqlite3_finalize( stmt );
   }
   sqlite3_close( db );   
}