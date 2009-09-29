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
//     Contributor(s):
//
//************************************************************************************************
#define _CRT_SECURE_NO_WARNINGS  // remove the new mircosoft security warnings

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "increadimail_convert.h"
#include "base64.h"

const char *ATTACHMENT = "----------[%ImFilePath%]----------";

void email_count( char *filename, int *email_total, int *deleted_emails ) {
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


void extract_eml_files( char *filename_data, char *eml_filename, int offset, unsigned int size ) {
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


void insert_attachments( char *eml_filename, char *attachments_path, char *final_email_filename ) {

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