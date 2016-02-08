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

#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <Shellapi.h>
#include <winuser.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "resource.h"
#include "increadimail_convert.h"
#include "about_dlg.h"
#include "winmain.h"

typedef enum {
   THREAD_NOT_STARTED = 0,
   THREAD_IN_PROGRESS = 1,
   THREAD_COMPLETED = 2,
} thread_status ;

HINSTANCE hThisInst;         // This instance
HWND global_hwnd;            // global window handle for the progress
thread_status email_thread;  // thread status

// Developer debug
// #define CONSOLE_OUTPUT 1

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int sShowCmd ) {
MSG msg;

#ifdef CONSOLE_OUTPUT
FILE *stream;
#endif

   hThisInst = hInstance;

   // Load the Fox icon
   LoadIcon( hInstance, MAKEINTRESOURCE( IDI_REYNARD ) );

#ifdef CONSOLE_OUTPUT
   // DEBUG INFORMATION TO CONSOLE*********
   AllocConsole();
   freopen_s(&stream,"conin$","r",stdin);
   freopen_s(&stream,"conout$","w",stdout);
   freopen_s(&stream,"conout$","w",stderr);
   //**************************************
#endif

   // Start the DialogBox
   DialogBox( hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC) Winload );

   while(GetMessage( &msg, NULL, 0, 0 ) ) {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
   }

   return (int) msg.wParam;
}


BOOL CALLBACK Winload( HWND hdwnd, UINT message, WPARAM wparam, LPARAM lparam ) {

// strings
char file_title[MAX_CHAR];
char window_title[MAX_CHAR];
char filter[MAX_CHAR];
char im_database_filename[MAX_CHAR];
char im_header_filename[MAX_CHAR];
char im_attachments_directory[MAX_CHAR];
char debug_str[MAX_CHAR];
char debug_str2[MAX_CHAR];
char version[5];

int e_count;  // email count
int d_count;  // deleted email count
static int blink = 0;  // blink status
int i;

int dialog_buttons[] = { IDOK, IDC_BROWSE, IDC_BROWSE2, IDC_CHECK1, IDC_EXPORT_FILE, IDC_EXPORT_DIRECTORY };

OPENFILENAME         openfile;
BROWSEINFO           bi;
LPITEMIDLIST         idlist;
ITEMIDLIST           idlistspace;
INITCOMMONCONTROLSEX cc;
HWND                 control;
static HANDLE        hThread;
DWORD                dwThreadId;
DWORD                ExitCode;
static DWORD         export_directory;

TCHAR tbuffer[256];
int tint = 256;
WIN32_FIND_DATA FindFileData;
HANDLE hFind;

   // Always Initialize Memory
   ZeroMemory( &file_title, sizeof( file_title ) );
   ZeroMemory( &im_database_filename, sizeof( im_database_filename ) );
   ZeroMemory( &im_header_filename, sizeof( im_header_filename ) );
   ZeroMemory( &im_attachments_directory, sizeof( im_attachments_directory ) );

   // check on the status of the email thread
   if( email_thread == THREAD_COMPLETED ) {
      KillTimer(hdwnd, 1);

      sprintf_s( debug_str2, MAX_CHAR, " " );
      SetDlgItemText( hdwnd, IDC_STATIC9, debug_str2 );            

      // reset the status of the thread
      email_thread = THREAD_NOT_STARTED;

      // enable all buttons again
      for( i = 0; i < 6; i++ ) {
         control = GetDlgItem( global_hwnd, dialog_buttons[i] );
         Button_Enable( control, 1 );
      }
      InvalidateRect( hdwnd, NULL, FALSE);  //  Redraw the Windows
   }

   switch( message ) {
      case WM_INITDIALOG:          
         // Setup progress bars
         ZeroMemory( &cc, sizeof( cc ) );
         cc.dwSize = sizeof( INITCOMMONCONTROLSEX );
         cc.dwICC  = ICC_PROGRESS_CLASS;
         SendDlgItemMessage( hdwnd, IDC_PROGRESS1, PBM_SETSTEP, (WPARAM) 1, 0 );

         //Default for buttons
         //SendDlgItemMessage( hdwnd, IDC_CHECK1, BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
         //SendDlgItemMessage( hdwnd, IDC_EXPORT_FILE, BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
         SendDlgItemMessage( hdwnd, IDC_EXPORT_DIRECTORY, BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
         
         // set this up for a second thread
         global_hwnd = hdwnd;

         // show stuff for the second progress bar
         control = GetDlgItem( global_hwnd, IDC_PROGRESS2 );
         ShowWindow( control, SW_SHOW );
         control = GetDlgItem( global_hwnd, IDC_OVERALL_PROGRESS );
         ShowWindow( control, SW_SHOW );
         control = GetDlgItem( global_hwnd, IDC_OVERALL_PERCENT );
         ShowWindow( control, SW_SHOW );

         // ok, this is should be easy
         // automatic search of IM database directory
         ZeroMemory( tbuffer, 256 );
         GetUserName( tbuffer, &tint );
         sprintf_s( im_header_filename, sizeof( im_header_filename ), "C:\\Documents and Settings\\%s\\Local Settings\\Application Data\\IM\\Identities\\*", tbuffer );
         hFind = FindFirstFile(im_header_filename, &FindFileData);  // should be .
         if( FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY && strcmp( FindFileData.cFileName, ".") == 0 ) {
            FindNextFile( hFind, &FindFileData );  // should be ..
            if( FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY && strcmp( FindFileData.cFileName, "..") == 0 ) {
               FindNextFile( hFind, &FindFileData );  // should be the real and only directory...
               if( FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY ) {
                  sprintf_s( im_header_filename, sizeof( im_header_filename ), "C:\\Documents and Settings\\%s\\Local Settings\\Application Data\\IM\\Identities\\%s\\Message Store", tbuffer, FindFileData.cFileName );
                  if( GetFileAttributes( im_header_filename ) == FILE_ATTRIBUTE_DIRECTORY ) {
                     SetDlgItemText( hdwnd, IDC_EDIT1, im_header_filename );
                  } 
                  // insert automatic attachment directory here
                  strcat_s( im_header_filename, MAX_CHAR, "\\Attachments");
                  if( GetFileAttributes( im_header_filename ) == FILE_ATTRIBUTE_DIRECTORY ) {
                     SetDlgItemText( hdwnd, IDC_EDIT2, im_header_filename );         
                  }
               }
            }
         }
         FindClose( hFind );
      return 1;

      case WM_TIMER:
         switch (wparam) {
         case 1:
            if( email_thread == THREAD_IN_PROGRESS ) {
               ZeroMemory( &debug_str2, sizeof( debug_str2 ) );
               blink = !blink;
               if( blink ) {
                  sprintf_s( debug_str2, MAX_CHAR, "*Converting*" );
                  SetDlgItemText( hdwnd, IDC_STATIC9, debug_str2 );
               } else {
                  sprintf_s( debug_str2, MAX_CHAR, " " );
                  SetDlgItemText( hdwnd, IDC_STATIC9, debug_str2 );            
               }
            } else {
               sprintf_s( debug_str2, MAX_CHAR, " " );
               SetDlgItemText( hdwnd, IDC_STATIC9, debug_str2 );
            }
         }
         return 1; 
      return 1;

      case WM_COMMAND:
         switch(LOWORD(wparam)) {

            case IDC_BROWSE:  // The database browse button
               export_directory = (int) SendDlgItemMessage( global_hwnd, IDC_EXPORT_DIRECTORY, BM_GETCHECK, 0, 0);

               if( export_directory ) {
                  ZeroMemory( &bi, sizeof( bi ) );
                  ZeroMemory( &idlist, sizeof( idlist ) );
                  strcpy_s( file_title, MAX_CHAR, "Select the database directory" );

                  bi.pszDisplayName = file_title;
                  bi.lpszTitle = file_title;
                  bi.pidlRoot = idlist;

                  idlist = &idlistspace;
                  idlist = SHBrowseForFolder( &bi );
                  SHGetPathFromIDList( idlist, im_database_filename );
                  SetDlgItemText( hdwnd, IDC_EDIT1, im_database_filename );

                  // insert automatic attachment directory here
                  strcat_s( im_database_filename, MAX_CHAR, "\\Attachments");
                  if( GetFileAttributes( im_database_filename ) == FILE_ATTRIBUTE_DIRECTORY ) {
                     SetDlgItemText( hdwnd, IDC_EDIT2, im_database_filename );         
                  }
               } else {
                  ZeroMemory( &window_title, sizeof( window_title ) );
                  ZeroMemory( &openfile, sizeof( openfile ) );
                  ZeroMemory( &filter, sizeof( filter ) );
                  ZeroMemory( &version, sizeof( version ) );

                  openfile.hwndOwner      = hdwnd;
                  openfile.lStructSize    = sizeof( openfile );
                  openfile.Flags          = OFN_READONLY;
                  openfile.nMaxFile       = sizeof( im_database_filename );
                  openfile.lpstrFileTitle = file_title;
                  openfile.lpstrFile      = &im_database_filename[0];
                  openfile.nMaxFileTitle  = sizeof( file_title );

                  strcpy_s( window_title, sizeof("Open Incredimail Database File"), "Open Incredimail Database File");
                  openfile.lpstrTitle = window_title;

                  strcpy_s( filter, sizeof("Incredimail Database *.imm\0*.imm\0\0"), "Incredimail Database *.imm\0*.imm\0\0" );
                  openfile.lpstrFilter = &filter[0];

                  strcpy_s( im_database_filename, sizeof("*.imm"), "*.imm" );

                  if( GetOpenFileName( &openfile ) == TRUE ) {
                     SetDlgItemText( hdwnd, IDC_EDIT1, im_database_filename );

                     // get the directory (reuse varible im_header_filename)
                     strncpy_s( im_header_filename, MAX_CHAR , im_database_filename, strlen( im_database_filename ) - strlen( openfile.lpstrFileTitle ) );
                     if( FindIncredimailVersion( im_header_filename ) == INCREDIMAIL_XE ) {
                        strncpy_s( im_header_filename, MAX_CHAR ,im_database_filename, strlen( im_database_filename ) - 3 );
                        strcat_s( im_header_filename, MAX_CHAR, "imh" );
                        email_count( im_header_filename, &e_count, &d_count );
                        SetDlgItemText( hdwnd, IDC_STATIC6, "Version: Incredimail XE" );
                     } else {
                        Incredimail_2_Email_Count( im_database_filename, &e_count, &d_count );
                        SetDlgItemText( hdwnd, IDC_STATIC6, "Version: Incredimail 2" );
                     }
                     sprintf_s( debug_str, MAX_CHAR, "Email Count: %d", e_count );
                     SetDlgItemText( hdwnd, IDC_ECOUNT, debug_str );
                     sprintf_s( debug_str, MAX_CHAR, "Deleted Emails: %d", d_count );
                     SetDlgItemText( hdwnd, IDC_STATIC8, debug_str );
                     sprintf_s( debug_str, MAX_CHAR, "Database Name: %s", im_database_filename );
                     SetDlgItemText( hdwnd, IDC_DATABASE_NAME, debug_str );

                     // insert automatic attachment directory here
                     strncpy_s( im_database_filename, MAX_CHAR, openfile.lpstrFile, strlen(openfile.lpstrFile)-strlen(openfile.lpstrFileTitle) );
                     strcat_s( im_database_filename, MAX_CHAR, "Attachments");
                     if( GetFileAttributes( im_header_filename ) == FILE_ATTRIBUTE_DIRECTORY ) {
                        SetDlgItemText( hdwnd, IDC_EDIT2, im_database_filename );         
                     }
                  }
               }
               return 1;

            case IDC_BROWSE2:  // the attachement browse button
               ZeroMemory( &bi, sizeof( bi ) );
               ZeroMemory( &idlist, sizeof( idlist ) );
               strcpy_s( file_title, MAX_CHAR, "Select the attachment directory" );

               bi.pszDisplayName = im_attachments_directory;
               bi.lpszTitle = file_title;
               bi.pidlRoot = idlist;

               idlist = &idlistspace;
               idlist = SHBrowseForFolder( &bi );
               SHGetPathFromIDList( idlist, im_attachments_directory );
               SetDlgItemText( hdwnd, IDC_EDIT2, im_attachments_directory );
               return 1;

            case IDOK:  // OK Button
               GetDlgItemText( hdwnd, IDC_EDIT1, (LPSTR) &im_database_filename, 256 );       // get the incredimail database name
               GetDlgItemText( hdwnd, IDC_EDIT2, (LPSTR) &im_attachments_directory, 256 );   // get the attachement directory name

               if( im_database_filename[0] != '\0' && im_attachments_directory[0] != '\0' ) {

                  export_directory = (int) SendDlgItemMessage( global_hwnd, IDC_EXPORT_DIRECTORY, BM_GETCHECK, 0, 0);
                  // execute a thread for processing emails
                  if( export_directory ) {
                     hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) process_email_directory, 0, 0, &dwThreadId );
                  } else {
                     hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) process_emails, 0, 0, &dwThreadId );
                  }
                  if( hThread ) {
                     email_thread = THREAD_IN_PROGRESS;

                     // disable all buttons
                     for( i = 0; i < 6; i++ ) {
                        control = GetDlgItem( global_hwnd, dialog_buttons[i] );
                        Button_Enable( control, 0 );
                     }
                  }
                  SetTimer( hdwnd, 1, 500, (TIMERPROC) NULL );  // Start the blinking
                  SendDlgItemMessage( hdwnd, IDOK, BN_DISABLE, 0, 0 );
               } else {
                  // display an error if the database or attachment directory is NULL
                  if( im_database_filename[0] == '\0' ) {
                     sprintf_s( debug_str, sizeof("Need Incredimail database filename"), "Need Incredimail database filename" );
                     MessageBox( hdwnd, debug_str, "Error!", MB_OK );
                  } else {
                     sprintf_s( debug_str, sizeof("Need attachment directory"), "Need attachment directory" );
                     MessageBox( hdwnd, debug_str, "Error!", MB_OK );
                  }
               }
            return 1;

            case IDC_EXPORT_DIRECTORY:
               // show stuff for the second progress bar
               control = GetDlgItem( global_hwnd, IDC_PROGRESS2 );
               ShowWindow( control, SW_SHOW );
               control = GetDlgItem( global_hwnd, IDC_OVERALL_PROGRESS );
               ShowWindow( control, SW_SHOW );
               control = GetDlgItem( global_hwnd, IDC_OVERALL_PERCENT );
               ShowWindow( control, SW_SHOW );
            return 1;

            case IDC_EXPORT_FILE:
               // hide stuff for the second progress bar
               control = GetDlgItem( global_hwnd, IDC_PROGRESS2 );
               ShowWindow( control, SW_HIDE );
               control = GetDlgItem( global_hwnd, IDC_OVERALL_PROGRESS );
               ShowWindow( control, SW_HIDE );
               control = GetDlgItem( global_hwnd, IDC_OVERALL_PERCENT );
               ShowWindow( control, SW_HIDE );
            return 1;

            case IDCANCEL:  // Cancel Button

               // End thread if in progress
               if( email_thread == THREAD_IN_PROGRESS ) {
                  GetExitCodeThread( hThread, &ExitCode );
                  TerminateThread( hThread, ExitCode );
               }
               CloseHandle( hThread );

               // End Dialog
               EndDialog( hdwnd, 0 );
               PostQuitMessage( 0 );
            return 1;

            case IDC_ABOUT:  // About button
                DialogBox( hThisInst, MAKEINTRESOURCE(IDD_ABOUT), NULL, (DLGPROC) About_Box );
            return 1;
         }
   }

   return 0;
}


void WINAPI process_emails() {

char im_header_filename[MAX_CHAR];
char im_database_filename[MAX_CHAR];
char im_attachments_directory[MAX_CHAR];
char debug_str[MAX_CHAR];
char new_eml_filename[MAX_CHAR];
char export_directory[MAX_CHAR];
char temp_path[MAX_CHAR];
char temp_filename[MAX_CHAR];

int e_count, d_count;
int deleted_email, export_all_email;
unsigned int offset, size;
int i, result_header, result_database, result_attachment, result_create_temp;
struct _stat buf;
float percent_complete;
int real_count = 0;
char *pdest;

enum INCREDIMAIL_VERSION incredimail_version;

   // Zero out the string names
   ZeroMemory( &im_header_filename, sizeof( im_header_filename ) );
   ZeroMemory( &im_database_filename, sizeof( im_database_filename ) );
   ZeroMemory( &im_attachments_directory, sizeof( im_attachments_directory ) );
   ZeroMemory( &new_eml_filename, sizeof( new_eml_filename ) );
   ZeroMemory( &export_directory, sizeof( export_directory ) );
   ZeroMemory( &temp_path, sizeof( temp_path ) );
   ZeroMemory( &temp_filename, sizeof( temp_filename ) );

   SendDlgItemMessage( global_hwnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0 );                 // reset the progress bar to 0%

   GetDlgItemText( global_hwnd, IDC_EDIT1, (LPSTR) &im_database_filename, 256 );       // get the incredimail database name
   GetDlgItemText( global_hwnd, IDC_EDIT2, (LPSTR) &im_attachments_directory, 256 );   // get the attachement directory name

   pdest = strrchr( im_database_filename, '\\' );
   strncpy_s( temp_path, MAX_CHAR, im_database_filename, strlen( im_database_filename ) - strlen( pdest ) );

   // get incredimail version
   incredimail_version = FindIncredimailVersion( temp_path );
   if( incredimail_version == INCREDIMAIL_XE ) {
      // get the header filename
      strncpy_s( im_header_filename, MAX_CHAR, im_database_filename, strlen( im_database_filename ) - 3 );
      strcat_s( im_header_filename, MAX_CHAR, "imh" );

      result_header     = _stat( im_header_filename, &buf );
   } else {
      result_header     = 0;
   }

   result_database   = _stat( im_database_filename, &buf );
   result_attachment = _stat( im_attachments_directory, &buf );

   // check if file or directory exists
   if( result_header != 0 || result_database != 0 || result_attachment != 0  ) {
      if( result_header != 0 ) {
         MessageBox( global_hwnd, "Can't open associated .imh file", "Error!", MB_OK );
      } else if( result_database != 0 ) {
         MessageBox( global_hwnd, "Can't open the Incredimail database file", "Error!", MB_OK );
      } else if( result_attachment != 0 ) {
         MessageBox( global_hwnd, "Can't open attachment directory", "Error!", MB_OK );
      }
   } else {
      // the export directory is based off of the database name
      strncpy_s( export_directory, MAX_CHAR, im_database_filename, strlen( im_database_filename ) - 4 );
      DeleteDirectory( export_directory );
      CreateDirectory( export_directory, NULL );
      strcat_s( export_directory, MAX_CHAR, "\\" );

      // set the email and deleted count to zero
      e_count = 0;
      d_count = 0;

      if( incredimail_version == INCREDIMAIL_XE ) {
         email_count( im_header_filename, &e_count, &d_count );
      } else {
         Incredimail_2_Email_Count( im_database_filename, &e_count, &d_count );
      }

      // get the state of the checkbox
      export_all_email = (int) SendDlgItemMessage( global_hwnd, IDC_CHECK1, BM_GETCHECK, 0, 0);

      // reset the progress bar
      SendDlgItemMessage( global_hwnd, IDC_PROGRESS1, PBM_SETRANGE, 0, (LPARAM) MAKELPARAM (0, e_count));

      // Get temp windows path
      result_create_temp = GetTempPath( sizeof( temp_path ), temp_path );

      for( i = 0; i < e_count; i++ ) {
         offset = 0;
         if( incredimail_version == INCREDIMAIL_XE ) {
            get_email_offset_and_size( im_header_filename, &offset, &size, i, e_count, &deleted_email );
         } else {
            Incredimail_2_Get_Email_Offset_and_Size( im_database_filename, &offset, &size, i, &deleted_email );
         }

         if( (export_all_email == BST_CHECKED) || !deleted_email ) {
            // setup the temp eml file name
            sprintf_s( new_eml_filename, MAX_CHAR, "email%d.eml", i );

            if( result_create_temp ) {
               GetTempFileName( temp_path, "eml", 0, temp_filename );
            }
            // extract the eml file in the temp directory
            extract_eml_files( im_database_filename, temp_filename, offset, size );

            ZeroMemory( export_directory, sizeof( export_directory ) );
            strcpy_s( export_directory, MAX_CHAR, im_database_filename);
            pdest = strrchr( export_directory, '.' );                  
            export_directory[strlen(export_directory) - strlen(pdest)] = '\0';
            
            strcat_s( export_directory, MAX_CHAR, "\\" );
            strcat_s( export_directory, MAX_CHAR, new_eml_filename );
            insert_attachments( temp_filename, im_attachments_directory, export_directory );
            DeleteFile( temp_filename );
         }
         // update the progress
         percent_complete =  ( ( (float) (i+1)/ (float) e_count) ) * 100;
         sprintf_s( debug_str, MAX_CHAR, "%d of %d (%0.0f%%)", i+1 ,e_count, percent_complete );
         SetDlgItemText( global_hwnd, IDC_XOFX, debug_str );
         SendDlgItemMessage( global_hwnd, IDC_PROGRESS1, PBM_STEPIT, 0, 0 );
         SendMessage( global_hwnd, WM_PAINT, 0, 0 );
      }
   
      // clean up, state it was done and delete the temp directory
      sprintf_s( debug_str, MAX_CHAR, "%d of %d DONE!",i ,e_count );
      SetDlgItemText( global_hwnd, IDC_XOFX, debug_str );

   }
   email_thread = THREAD_COMPLETED;
}


void WINAPI process_email_directory() {
char temp_file_listing[MAX_CHAR];
char im_database_filename[MAX_CHAR];
char im_header_filename[MAX_CHAR];
char im_attachments_directory[MAX_CHAR];
char export_directory[MAX_CHAR];
char debug_str[MAX_CHAR];
char new_eml_filename[MAX_CHAR];
char temp_path[MAX_CHAR];
char temp_filename[MAX_CHAR];

HANDLE inputfile;

int read_length = 1;
int e_count, d_count, i, j, total_count;
int result_database, result_attachment;
int deleted_email, export_all_email;
unsigned int offset, size;
float percent_complete;

DWORD result_create_temp;
struct _stat buf;

char *pdest;
enum INCREDIMAIL_VERSION incredimail_version;

   // Zero out the string names
   ZeroMemory( temp_file_listing, sizeof( temp_file_listing ) );
   ZeroMemory( im_attachments_directory, sizeof( im_attachments_directory ) );
   ZeroMemory( im_header_filename, sizeof( im_header_filename ) );
   ZeroMemory( im_database_filename, sizeof( im_database_filename ) );
   ZeroMemory( export_directory, sizeof( export_directory ) );
   ZeroMemory( temp_path, sizeof( temp_path ) );
   j = 0;

   GetDlgItemText( global_hwnd, IDC_EDIT1, im_database_filename, 256 );       // get the incredimail database name
   GetDlgItemText( global_hwnd, IDC_EDIT2, im_attachments_directory, 256 );   // get the attachement directory name

   result_database   = _stat( im_database_filename,     &buf );
   result_attachment = _stat( im_attachments_directory, &buf );

   if( result_database != 0 || result_attachment != 0 ) {
      if( result_database != 0 ) {
         MessageBox( global_hwnd, "Invalid Database Directory", "Error!", MB_OK );
      } else if( result_attachment != 0 ) {
         MessageBox( global_hwnd, "Can't open attachment directory", "Error!", MB_OK );
      }   
   } else {

      total_count = FindDatabaseFiles( im_database_filename, temp_file_listing );
	  if (total_count < 0)
		  return;

      // set the progress bar 2
      SendDlgItemMessage( global_hwnd, IDC_PROGRESS2, PBM_SETRANGE, 0, (LPARAM) MAKELPARAM (0, total_count));
      SendDlgItemMessage( global_hwnd, IDC_PROGRESS2, PBM_SETSTEP, 1, 0 );
      SendDlgItemMessage( global_hwnd, IDC_PROGRESS2, PBM_SETPOS, 0, 0 );

      inputfile  = CreateFile(temp_file_listing, GENERIC_READ, 0x0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	  if (inputfile == INVALID_HANDLE_VALUE) {
		  MessageBox(global_hwnd, "Can't open temporary file", "Error!", MB_OK);
		  return;
	  }

      do {
         ZeroMemory( im_database_filename, sizeof( im_database_filename ) );
         read_length = ReadOneLine( inputfile, im_database_filename, MAX_CHAR );
         // cleaning up the line feeds from FindDatabaseFiles function
         i = (int) strlen( im_database_filename );
         im_database_filename[i-2] = 0;

         if( read_length != 0 ) {
            SendDlgItemMessage( global_hwnd, IDC_PROGRESS2, PBM_STEPIT, 0, 0 );                 // set the overall progress bar
            SendDlgItemMessage( global_hwnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0 );                 // reset the progress bar to 0%

            j++;
            percent_complete =  ( ( (float) (j)/ (float) total_count) ) * 100;
            sprintf_s( debug_str, MAX_CHAR, "%d of %d (%0.0f%%)", j ,total_count, percent_complete );
            SetDlgItemText( global_hwnd, IDC_OVERALL_PERCENT, debug_str );

            // get incredimail version
            pdest = strrchr( im_database_filename, '\\' );
            strncpy_s( temp_path, sizeof(temp_path), im_database_filename, strlen( im_database_filename ) - strlen( pdest ) );
            incredimail_version = FindIncredimailVersion( temp_path );

            if( incredimail_version == INCREDIMAIL_XE ) {
               // get the header filename
               strncpy_s( im_header_filename, MAX_CHAR, im_database_filename, strlen(im_database_filename) - 4 );
               strcat_s( im_header_filename, MAX_CHAR, ".imh" );
            }

            // the export directory is based off of the database name
            strcpy_s( export_directory, MAX_CHAR, im_database_filename );
            pdest = strrchr( export_directory, '.' );
            if( pdest != 0 ) {
               export_directory[strlen(export_directory) - strlen(pdest)] = '\0';
            }

            DeleteDirectory( export_directory );
            CreateDirectory( export_directory, NULL );
            strcat_s( export_directory, MAX_CHAR, "\\" );

            // set the email and deleted count to zero
            e_count = 0;
            d_count = 0;

            if( incredimail_version == INCREDIMAIL_XE ) {
               email_count( im_header_filename, &e_count, &d_count );
               SetDlgItemText( global_hwnd, IDC_STATIC6, "Version: Incredimail XE" );
            } else {
               Incredimail_2_Email_Count( im_database_filename, &e_count, &d_count );
               SetDlgItemText( global_hwnd, IDC_STATIC6, "Version: Incredimail 2" );
            }

            sprintf_s( debug_str, MAX_CHAR, "Email Count: %d", e_count );
            SetDlgItemText( global_hwnd, IDC_ECOUNT, debug_str );
            sprintf_s( debug_str, MAX_CHAR, "Deleted Emails: %d", d_count );
            SetDlgItemText( global_hwnd, IDC_STATIC8, debug_str );

            sprintf_s( debug_str, MAX_CHAR, "Database: %s", im_database_filename );
            pdest = strrchr( debug_str, '\\' );
            sprintf_s( debug_str, MAX_CHAR, "Database: %s", &pdest[1] );
            SetDlgItemText( global_hwnd, IDC_DATABASE_NAME, debug_str );

            // get the state of the checkbox
            export_all_email = (int) SendDlgItemMessage( global_hwnd, IDC_CHECK1, BM_GETCHECK, 0, 0);

            // reset the progress bar
            SendDlgItemMessage( global_hwnd, IDC_PROGRESS1, PBM_SETRANGE, 0, (LPARAM) MAKELPARAM (0, e_count));

            // Get temp windows path
            result_create_temp = GetTempPath( sizeof( temp_path ), temp_path );

            for( i = 0; i < e_count; i++ ) {
               offset = 0;
               if( incredimail_version == INCREDIMAIL_XE ) {
                  get_email_offset_and_size( im_header_filename, &offset, &size, i, e_count, &deleted_email );
               } else {
                  Incredimail_2_Get_Email_Offset_and_Size( im_database_filename, &offset, &size, i, &deleted_email );
               }

               if( (export_all_email == BST_CHECKED) || !deleted_email ) {
                  // setup the temp eml file name
                  sprintf_s( new_eml_filename, MAX_CHAR, "email%d.eml", i );

                  if( result_create_temp ) {
                     GetTempFileName( temp_path, "eml", 0, temp_filename );
                  }
                  // extract the eml file in the temp directory
                  extract_eml_files( im_database_filename, temp_filename, offset, size );

                  ZeroMemory( export_directory, sizeof( export_directory ) );
                  strcpy_s( export_directory, MAX_CHAR, im_database_filename);
                  pdest = strrchr( export_directory, '.' );
                  if( pdest != 0 ) {
                     export_directory[strlen(export_directory) - strlen(pdest)] = '\0';
                  }

                  strcat_s( export_directory, MAX_CHAR, "\\" );
                  strcat_s( export_directory, MAX_CHAR, new_eml_filename );
                  insert_attachments( temp_filename, im_attachments_directory, export_directory );
                  DeleteFile( temp_filename );
               }
               
               // update the progress
               percent_complete =  ( ( (float) (i+1)/ (float) e_count) ) * 100;
               sprintf_s( debug_str, MAX_CHAR, "%d of %d (%0.0f%%)", i+1 ,e_count, percent_complete );
               SetDlgItemText( global_hwnd, IDC_XOFX, debug_str );
 
               SendDlgItemMessage( global_hwnd, IDC_PROGRESS1, PBM_STEPIT, 0, 0 );
               SendMessage( global_hwnd, WM_PAINT, 0, 0 );
            }
         }
      } while( read_length != 0 );
      CloseHandle( inputfile );
   }  
   email_thread = THREAD_COMPLETED;
}