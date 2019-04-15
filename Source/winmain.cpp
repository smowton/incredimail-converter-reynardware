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

#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <Shellapi.h>
#include <Shlobj.h>
#include <winuser.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <map>
#include <string>
#include <vector>

#include "resource.h"
#include "increadimail_convert.h"
#include "about_dlg.h"
#include "winmain.h"
#include "sqlite3.h"

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
char im_database_filename[MAX_CHAR];
char im_header_filename[MAX_CHAR];
wchar_t im_header_filename_w[MAX_CHAR];
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

int tint = 256;
WIN32_FIND_DATAW FindFileData;
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

		 PWSTR appDataPath;
		 SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appDataPath);
		 if (!appDataPath)
			 return 1;
		 wcscpy_s(im_header_filename_w, appDataPath);
		 wcscat_s(im_header_filename_w, L"\\IM\\Identities\\*");
		 
		 
         hFind = FindFirstFileW(im_header_filename_w, &FindFileData);  // should be .
         if( FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY && wcscmp( FindFileData.cFileName, L".") == 0 ) {
            FindNextFileW( hFind, &FindFileData );  // should be ..
            if( FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY && wcscmp( FindFileData.cFileName, L"..") == 0 ) {
               FindNextFileW( hFind, &FindFileData );  // should be the real and only directory...
               if( FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY ) {
				  wchar_t storename[MAX_CHAR];
				  swprintf_s(storename, L"%s\\IM\\Identities\\%s\\Message Store", appDataPath, FindFileData.cFileName);
                  if( GetFileAttributesW( storename ) == FILE_ATTRIBUTE_DIRECTORY ) {
                     SetDlgItemTextW( hdwnd, IDC_EDIT1, storename );
                  } 
                  // insert automatic attachment directory here
                  wcscat_s( storename, L"\\Attachments");
                  if( GetFileAttributesW( storename ) == FILE_ATTRIBUTE_DIRECTORY ) {
                     SetDlgItemTextW( hdwnd, IDC_EDIT2, storename );         
                  }
               }
            }
         }
		 CoTaskMemFree(appDataPath);
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
                  ZeroMemory( &version, sizeof( version ) );

                  openfile.hwndOwner      = hdwnd;
                  openfile.lStructSize    = sizeof( openfile );
                  openfile.Flags          = OFN_READONLY;
                  openfile.nMaxFile       = sizeof( im_database_filename );
                  openfile.lpstrFileTitle = file_title;
                  openfile.lpstrFile      = im_database_filename;
                  openfile.nMaxFileTitle  = sizeof( file_title );

                  strcpy_s( window_title, sizeof(window_title), "Open Incredimail Database File");
                  openfile.lpstrTitle = window_title;
				  const char* filter = "Incredimail Database\0*.imm;*.db\0";
                  openfile.lpstrFilter = filter;

                  if( GetOpenFileName( &openfile ) == TRUE ) {
					 enum INCREDIMAIL_VERSIONS version = FindIncredimailVersion(im_database_filename);
					 if(version == INCREDIMAIL_XE) {
                        strncpy_s( im_header_filename, MAX_CHAR ,im_database_filename, strlen( im_database_filename ) - 3 );
                        strcat_s( im_header_filename, MAX_CHAR, "imh" );
                        email_count( im_header_filename, &e_count, &d_count );
                        SetDlgItemText( hdwnd, IDC_STATIC6, "Version: Incredimail XE" );
                     } else if(version == INCREDIMAIL_2) {
                        Incredimail_2_Email_Count(im_database_filename, &e_count, &d_count);
                        SetDlgItemText( hdwnd, IDC_STATIC6, "Version: Incredimail 2" );
					 } else if (version == INCREDIMAIL_2_MAILDIR) {
						Incredimail_2_Maildir_Email_Count(im_database_filename, &e_count, &d_count);
						SetDlgItemText(hdwnd, IDC_STATIC6, "Version: Incredimail 2 Maildir");
					 } else {
						SetDlgItemText(hdwnd, IDC_STATIC6, "No database found");
						return 1;
					 }
                     sprintf_s( debug_str, MAX_CHAR, "Email Count: %d", e_count );
                     SetDlgItemText( hdwnd, IDC_ECOUNT, debug_str );
                     sprintf_s( debug_str, MAX_CHAR, "Deleted Emails: %d", d_count );
                     SetDlgItemText( hdwnd, IDC_STATIC8, debug_str );
                     sprintf_s( debug_str, MAX_CHAR, "Database Name: %s", im_database_filename );
                     SetDlgItemText( hdwnd, IDC_DATABASE_NAME, debug_str );

					 SetDlgItemText(hdwnd, IDC_EDIT1, im_database_filename);

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

struct im2_maildir_folder {
	
	struct im2_maildir_folder* parent;
	std::vector<struct im2_maildir_folder*> children;
	std::string id;
	std::string foldername;
	std::string account;

	im2_maildir_folder() : parent(0) {}

	bool create_directories(const std::string& prefix, std::map<std::string, std::string>& container_to_dir) {

		std::string path = prefix + "\\" + foldername;
		if ((!CreateDirectory(path.c_str(), NULL)) && GetLastError() != ERROR_ALREADY_EXISTS)
			return false;
		container_to_dir[id] = path;
		for (std::vector<struct im2_maildir_folder*>::iterator it = children.begin(), itend = children.end(); it != itend; ++it)
			(*it)->create_directories(path, container_to_dir);
		return true;

	}

	std::string get_unique_account_name() const {
		if (account.length() != 0)
			return account;

		std::string unique_account_name;
		for (const auto child : children) {
			std::string child_account = child->get_unique_account_name();
			if (child_account.length() != 0) {
				if (unique_account_name.length() == 0)
					unique_account_name = child_account;
				else if (unique_account_name != child_account) {
					unique_account_name = "";
					break;
				}
			}
		}

		return unique_account_name;
	}

};

void WINAPI process_emails2(const char* im_database_filename, const char* im_attachments_directory);

void WINAPI process_emails() {
	
	char im_database_filename[MAX_CHAR];
	char im_attachments_directory[MAX_CHAR];

	GetDlgItemText(global_hwnd, IDC_EDIT1, (LPSTR)&im_database_filename, 256);       // get the incredimail database name
	GetDlgItemText(global_hwnd, IDC_EDIT2, (LPSTR)&im_attachments_directory, 256);   // get the attachement directory name

	process_emails2(im_database_filename, im_attachments_directory);

}

void WINAPI process_emails2(const char* im_database_filename, const char* im_attachments_directory) {

char im_header_filename[MAX_CHAR];
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
const char *pdest;

enum INCREDIMAIL_VERSIONS incredimail_version;

   // Zero out the string names
   ZeroMemory( &im_header_filename, sizeof( im_header_filename ) );
   ZeroMemory( &new_eml_filename, sizeof( new_eml_filename ) );
   ZeroMemory( &export_directory, sizeof( export_directory ) );
   ZeroMemory( &temp_path, sizeof( temp_path ) );
   ZeroMemory( &temp_filename, sizeof( temp_filename ) );

   SendDlgItemMessage( global_hwnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0 );                 // reset the progress bar to 0%

   // get incredimail version
   incredimail_version = FindIncredimailVersion( im_database_filename );
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
	  const char* extsep = strrchr(im_database_filename, '.');
	  const char* dirsep = strrchr(im_database_filename, '\\');
	  if (incredimail_version == INCREDIMAIL_2_MAILDIR) {
		  if (dirsep == NULL) {
			  strcpy_s(export_directory, im_database_filename);
			  strcat_s(export_directory, ".exported");
		  }
		  else {
			  strncpy_s(export_directory, MAX_CHAR, im_database_filename, (dirsep - im_database_filename));
			  strcat_s(export_directory, "\\Exported Incredimail Messages");
		  }
      }
	  else {
		   if (extsep != NULL && (dirsep == NULL || dirsep < extsep))
			   strncpy_s(export_directory, MAX_CHAR, im_database_filename, (extsep - im_database_filename));
		   else {
			   strcpy_s(export_directory, im_database_filename);
			   strcat_s(export_directory, ".exported");
		   }
	  }
      DeleteDirectory( export_directory );
	  if (!CreateDirectory(export_directory, NULL)) {
		  std::string error = "Can't create " + std::string(export_directory);
		  MessageBox(global_hwnd, error.c_str(), "Error!", MB_OK);
		  return;
	  }


      strcat_s( export_directory, MAX_CHAR, "\\" );

      // set the email and deleted count to zero
      e_count = 0;
      d_count = 0;

      if( incredimail_version == INCREDIMAIL_XE ) {
         email_count( im_header_filename, &e_count, &d_count );
      } else if(incredimail_version == INCREDIMAIL_2) {
         Incredimail_2_Email_Count( im_database_filename, &e_count, &d_count );
	  } else {
		 Incredimail_2_Maildir_Email_Count(im_database_filename, &e_count, &d_count);
	  }

      // get the state of the checkbox
      export_all_email = (int) SendDlgItemMessage( global_hwnd, IDC_CHECK1, BM_GETCHECK, 0, 0);

      // reset the progress bar
      SendDlgItemMessage( global_hwnd, IDC_PROGRESS1, PBM_SETRANGE, 0, (LPARAM) MAKELPARAM (0, e_count));

      // Get temp windows path
      result_create_temp = GetTempPath( sizeof( temp_path ), temp_path );

	  std::map<std::string, im2_maildir_folder> containers;
	  std::map<std::string, std::string> container_to_dir;
	  sqlite3 *db = NULL;
	  sqlite3_stmt *stmt = NULL;

	  if (incredimail_version == INCREDIMAIL_2_MAILDIR) {
		  
		  if (sqlite3_open_v2(im_database_filename, &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
			  MessageBox(global_hwnd, "Can't open database", "Error!", MB_OK);
			  return;
		  }

		  if (sqlite3_prepare_v2(db, "select ContainerID, ParentContainerID, Label from Containers", -1, &stmt, NULL) != SQLITE_OK) {
			  sqlite3_close(db);
			  MessageBox(global_hwnd, "Containers query failed", "Error!", MB_OK);
			  return;
		  }

		  while (sqlite3_step(stmt) == SQLITE_ROW) {

			  const char* thisid = (const char*)sqlite3_column_text(stmt, 0);
			  const char* parentid = (const char*)sqlite3_column_text(stmt, 1);
			  const char* label = (const char*)sqlite3_column_text(stmt, 2);
			  
			  im2_maildir_folder& thisfolder = containers[thisid];
			  if (parentid && strlen(parentid)) {
				  thisfolder.parent = &containers[parentid];
				  thisfolder.parent->children.push_back(&thisfolder);
			  }
			  thisfolder.id = thisid;
			  thisfolder.foldername = label;

			  // Figure out the owning account name from message headers:
			  std::string name_query = "select account, count(*) from headers where containerid = \"" + thisfolder.id + "\" group by account order by count(*) desc limit 1";
			  sqlite3_stmt *name_statement = NULL;

			  if (sqlite3_prepare_v2(db, name_query.c_str(), -1, &name_statement, NULL) != SQLITE_OK) {
				  sqlite3_close(db);
				  MessageBox(global_hwnd, "Account name query failed", "Error!", MB_OK);
				  return;
			  }

			  if (sqlite3_step(name_statement) == SQLITE_ROW) {
				  thisfolder.account = (const char *)sqlite3_column_text(name_statement, 0);
			  }

			  sqlite3_finalize(name_statement);

		  }

		  int rootidx = 0;
		  for (std::map<std::string, im2_maildir_folder>::iterator it = containers.begin(), itend = containers.end(); it != itend; ++it) {

			  if (it->second.parent)
				  continue;

			  // Try to inherit an account name:
			  std::string unique_account_name = it->second.get_unique_account_name();

			  if (unique_account_name.length() != 0) {
				  it->second.foldername = "Account " + std::to_string(++rootidx) + " (" + unique_account_name + ")";
			  }
			  else {
				  it->second.foldername = "Unnamed account " + std::to_string(++rootidx);
			  }

			  if (!it->second.create_directories(export_directory, container_to_dir)) {
				  MessageBox(global_hwnd, "Failed to create some message directory", "Error!", MB_OK);
				  sqlite3_close(db);
				  return;
			  }

		  }

		  sqlite3_reset(stmt);
		  sqlite3_finalize(stmt);

		  if (sqlite3_prepare_v2(db, "select HeaderID, ContainerID, Location, Deleted from Headers where Location != \"\"", -1, &stmt, NULL) != SQLITE_OK) {
			  sqlite3_close(db);
			  MessageBox(global_hwnd, "Headers query failed", "Error!", MB_OK);
			  return;
		  }

	  }

	  std::string messages_dir = im_database_filename;
	  size_t truncate_off = messages_dir.rfind('\\');
	  if (truncate_off == std::string::npos)
		  truncate_off = 0;
	  messages_dir.erase(truncate_off);
	  messages_dir.append("\\Messages");

	  std::size_t n_failures = 0;

      for( i = 0; i < e_count; i++ ) {
         offset = 0;

		 std::string message_attachment_dir;

		 sprintf_s(new_eml_filename, MAX_CHAR, "email%d.eml", i);
		 GetTempFileName(temp_path, "eml", 0, temp_filename);
		 std::string target_path;

		 if (incredimail_version == INCREDIMAIL_2_MAILDIR) {
			 
			 if (sqlite3_step(stmt) != SQLITE_ROW) {
				 MessageBox(global_hwnd, "Unexpected failure to fetch a header row!", "Error!", MB_OK);
				 sqlite3_close(db);
				 return;
			 }

			 if (export_all_email != BST_CHECKED && sqlite3_column_int(stmt, 3))
				 continue;

			 const char* container = (const char*)sqlite3_column_text(stmt, 1);
			 std::map<std::string, std::string>::iterator findit = container_to_dir.find(container);
			 if (findit == container_to_dir.end()) {
				 MessageBox(global_hwnd, "Message with unknown container?", "Error!", MB_OK);
				 sqlite3_close(db);
				 return;
			 }

			 target_path = findit->second + "\\" + new_eml_filename;

			 const char* headerid = (const char*)sqlite3_column_text(stmt, 0);
			 const char* subfolder = (const char*)sqlite3_column_text(stmt, 2);

			 std::string source_path = messages_dir + "\\" + subfolder + "\\" + headerid + "\\msg.iml";
			 message_attachment_dir = messages_dir + "\\" + subfolder + "\\" + headerid + "\\Attachments";

			 if (!CopyFile(source_path.c_str(), temp_filename, FALSE)) {
				 ++n_failures;
				 continue;
			 }

		 }
		 else {

			 if (incredimail_version == INCREDIMAIL_XE) {
				 get_email_offset_and_size(im_header_filename, &offset, &size, i, e_count, &deleted_email);
			 }
			 else {
				 Incredimail_2_Get_Email_Offset_and_Size(im_database_filename, &offset, &size, i, &deleted_email);
			 }

			 if ((export_all_email != BST_CHECKED) && deleted_email)
				 continue;

			 // extract the eml file in the temp directory
			 extract_eml_files(im_database_filename, temp_filename, offset, size);

			 ZeroMemory(export_directory, sizeof(export_directory));
			 strcpy_s(export_directory, MAX_CHAR, im_database_filename);
			 pdest = strrchr(export_directory, '.');
			 export_directory[strlen(export_directory) - strlen(pdest)] = '\0';

			 strcat_s(export_directory, MAX_CHAR, "\\");
			 strcat_s(export_directory, MAX_CHAR, new_eml_filename);

			 target_path = std::string(export_directory);
			 message_attachment_dir = im_attachments_directory;

		 }

		 im_to_eml(temp_filename, message_attachment_dir.c_str(), target_path.c_str());
		 DeleteFile(temp_filename);

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

	  if (n_failures != 0) {
		  std::string complaint = "Couldn't find " + std::to_string(n_failures) + " out of " + std::to_string(e_count) + " expected message files";
		  MessageBox(global_hwnd, complaint.c_str(), "Error!", MB_OK);
	  }

	  std::string result_msg = "Exported messages from " + std::string(im_database_filename) + " to " + std::string(export_directory);
	  MessageBox(global_hwnd, result_msg.c_str(), "Export Complete", MB_OK);

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
enum INCREDIMAIL_VERSIONS incredimail_version;

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

	  incredimail_version = FindIncredimailVersion(im_database_filename);
      
	  if (incredimail_version == INCREDIMAIL_2_MAILDIR) {
		  std::string messagestore_path = std::string(im_database_filename) + "\\MessageStore.db";
		  process_emails2(messagestore_path.c_str(), im_attachments_directory);
		  return;
	  }
	  else if (incredimail_version == INCREDIMAIL_VERSION_UNKNOWN) {
		  MessageBox(global_hwnd, "No .imm files and no MessageStore.db found within given directory", "Error!", MB_OK);
		  exit(1);
	  }

	  total_count = FindDatabaseFiles(im_database_filename, temp_file_listing);
	  
      // set the progress bar 2
      SendDlgItemMessage( global_hwnd, IDC_PROGRESS2, PBM_SETRANGE, 0, (LPARAM) MAKELPARAM (0, total_count));
      SendDlgItemMessage( global_hwnd, IDC_PROGRESS2, PBM_SETSTEP, 1, 0 );
      SendDlgItemMessage( global_hwnd, IDC_PROGRESS2, PBM_SETPOS, 0, 0 );

      inputfile  = CreateFile(temp_file_listing, GENERIC_READ, 0x0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	  if (inputfile == INVALID_HANDLE_VALUE) {
		  MessageBox(global_hwnd, "Can't open temporary file", "Error!", MB_OK);
		  return;
	  }

	  std::string first_exported_file_description;

      do {
         ZeroMemory( im_database_filename, sizeof( im_database_filename ) );
         read_length = ReadOneLine( inputfile, im_database_filename, MAX_CHAR );
         // cleaning up the line feeds from FindDatabaseFiles function
         i = (int) strlen( im_database_filename );
		 if (i < 2) {
			 char msg[MAX_CHAR];
			 snprintf(msg, MAX_CHAR, "Ignoring invalid filename %s", im_database_filename);
			 MessageBox(global_hwnd, msg, "Error!", MB_OK);
			 continue;
		 }

         im_database_filename[i-2] = 0;

         if( read_length != 0 ) {
            SendDlgItemMessage( global_hwnd, IDC_PROGRESS2, PBM_STEPIT, 0, 0 );                 // set the overall progress bar
            SendDlgItemMessage( global_hwnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0 );                 // reset the progress bar to 0%

            j++;
            percent_complete =  ( ( (float) (j)/ (float) total_count) ) * 100;
            sprintf_s( debug_str, MAX_CHAR, "%d of %d (%0.0f%%)", j ,total_count, percent_complete );
            SetDlgItemText( global_hwnd, IDC_OVERALL_PERCENT, debug_str );

            // get incredimail version
            incredimail_version = FindIncredimailVersion( im_database_filename );
			if (incredimail_version == INCREDIMAIL_VERSION_UNKNOWN) {
				char msg[CHAR_MAX];
				sprintf_s(msg, CHAR_MAX, "Directory %s doesn't match a known Incredimail version", temp_path);
				continue;
			}

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

			if (first_exported_file_description.size() == 0) {
				first_exported_file_description = std::string(im_database_filename) + " was exported to " + std::string(export_directory);
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
                  im_to_eml( temp_filename, im_attachments_directory, export_directory );
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

	  if (total_count > 0) {
		  std::string completion_msg = "Exported " + std::to_string(total_count) +
			  " database files. The results are side-by-side with the database files: for example, " +
			  first_exported_file_description;
		  MessageBox(global_hwnd, completion_msg.c_str(), "Export complete", MB_OK);
	  }
   }  
   email_thread = THREAD_COMPLETED;
}

int DeleteDirectory(const char *sPath) {
	return system((std::string("rd /s /q \"") + sPath + "\"").c_str());
}
