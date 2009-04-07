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

#ifndef __INCREDIMAIL_CONVERT__
#define __INCREDIMAIL_CONVERT__

#define S_LEN 1024*2
#define MAX_CHAR 256

static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//  TO BE REMOVED
/*
typedef struct {
   unsigned char unknown_header[0x26];
   unsigned int  from_size;
   unsigned char from_email[S_LEN];
   unsigned int  to_size;
   unsigned char to_email[S_LEN];
   unsigned int  subject_size;
   unsigned char subject[S_LEN];
   unsigned char unknown_header_2[0x0C];
   unsigned int  pop_size;
   unsigned char pop_name[S_LEN];
   unsigned char unknown_header_3[0x06];
   unsigned int  file_size;
   unsigned char unknown_header_4[0x08];
} record;
*/

int email_count( char * );
void get_email_offset_and_size( char *, unsigned int *, unsigned int *, int, int );
void encodeblock( unsigned char in[3], unsigned char out[4], int len );
void encode( FILE *infile, FILE *outfile, int linesize );
void extract_eml_files( char *filename_data, char *, int offset, unsigned int size );
void insert_attachments( char *eml_filename, char *attachments_path, char * );
BOOL DeleteDirectory(const TCHAR*);

#endif