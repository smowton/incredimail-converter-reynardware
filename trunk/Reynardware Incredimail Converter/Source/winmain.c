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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <Shellapi.h>
#include <winuser.h>
#include <shlobj.h>
#include <richedit.h>

#include "resource.h"
#include "Increadimail_Convert.h"

BOOL CALLBACK Winload( HWND, UINT, WPARAM, LPARAM );
BOOL CALLBACK About_Box( HWND, UINT, WPARAM, LPARAM );

HINSTANCE hThisInst;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int sShowCmd ) {
MSG msg;

   hThisInst = hInstance;

   DialogBox( hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC) Winload );

   while(GetMessage( &msg, NULL, 0, 0 ) ) {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
   }

   return msg.wParam;
}


BOOL CALLBACK Winload( HWND hdwnd, UINT message, WPARAM wparam, LPARAM lparam ) {
char im_database_filename[256];
char im_header_filename[256];
char im_attachments_directory[256];
char file_title[256];
char debug_str[256];
char window_title[256];
char filter[256];
char export_directory[256];
char eml_filename[256];
char new_eml_filename[256];
char *token;

int e_count;
unsigned int offset, size;
int i;

OPENFILENAME openfile;
BROWSEINFO bi;
LPITEMIDLIST idlist;
ITEMIDLIST idlistspace;
INITCOMMONCONTROLSEX cc;

  ZeroMemory( &cc, sizeof( cc ) );
  ZeroMemory( &openfile, sizeof( openfile ) );
  ZeroMemory( &bi, sizeof( bi ) );
  ZeroMemory( &im_database_filename, sizeof( im_database_filename ) );
  ZeroMemory( &im_header_filename, sizeof( im_header_filename ) );
  ZeroMemory( &im_attachments_directory, sizeof( im_attachments_directory ) );
  ZeroMemory( &file_title, sizeof( file_title ) );
  ZeroMemory( &idlist, sizeof( idlist ) );
  ZeroMemory( &debug_str, sizeof( debug_str ) );
  ZeroMemory( &eml_filename, sizeof( eml_filename ) );
  ZeroMemory( &new_eml_filename, sizeof( new_eml_filename ) );
  ZeroMemory( &window_title, sizeof( window_title ) );
  ZeroMemory( &filter, sizeof( filter ) );
  ZeroMemory( &export_directory, sizeof( export_directory ) );

  openfile.hwndOwner      = hdwnd;
  openfile.lStructSize    = sizeof( openfile );
  openfile.Flags          = OFN_READONLY;
  openfile.nMaxFile       = sizeof( im_database_filename );
  openfile.lpstrFileTitle = file_title;
  openfile.lpstrFile      = &im_database_filename[0];
  openfile.nMaxFileTitle  = sizeof( file_title );

  strcpy( window_title, "Open Incredimail Database File");
  openfile.lpstrTitle = window_title;

  strcpy( filter, "*.imm\0*.imm");
  openfile.lpstrFilter = &filter[0];

  cc.dwSize = sizeof( INITCOMMONCONTROLSEX );
  cc.dwICC  = ICC_PROGRESS_CLASS;

   switch( message ) {
      case WM_INITDIALOG:          
         SendDlgItemMessage( hdwnd, IDC_PROGRESS1, PBM_SETSTEP, (WPARAM) 1, 0 );
      return 1;

      case WM_COMMAND:
         switch(LOWORD(wparam)) {

            case IDC_BROWSE:
               if( GetOpenFileName( &openfile ) == TRUE ) {
                  SetDlgItemText( hdwnd, IDC_EDIT1, im_database_filename );
               }
               strncpy( im_header_filename, im_database_filename, strlen( im_database_filename ) - 3 );
               strcat( im_header_filename, "imh" );
               e_count = email_count( im_header_filename );
               sprintf( debug_str, "Email Count: %d", e_count );
               SetDlgItemText( hdwnd, IDC_ECOUNT, debug_str );
               return 1;

            case IDC_BROWSE2:
               strcpy( file_title, "Select the attachment directory" );

               bi.pszDisplayName = im_attachments_directory;
               bi.lpszTitle = file_title;
               bi.pidlRoot = idlist;

               idlist = &idlistspace;
               idlist = SHBrowseForFolder( &bi );
               SHGetPathFromIDList( idlist, im_attachments_directory );
               SetDlgItemText( hdwnd, IDC_EDIT2, im_attachments_directory );
               SendDlgItemMessage( hdwnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0 );
               return 1;

            case IDOK:
               SendDlgItemMessage( hdwnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0 );
               GetDlgItemText( hdwnd, IDC_EDIT1, &im_database_filename, 256 );
               GetDlgItemText( hdwnd, IDC_EDIT2, &im_attachments_directory, 256 );
               strncpy( im_header_filename, im_database_filename, strlen( im_database_filename ) - 3 );
               strcat( im_header_filename, "imh" );

               // EXPORT DIRECTORY HERE
               strncpy( export_directory, im_header_filename, strlen( im_database_filename ) - 4 );
               // TODO: Create a delete directory function
               DeleteFile( export_directory );
               CreateDirectory( export_directory, NULL );
               strcat( export_directory, "\\" );

               e_count = email_count( im_header_filename );
               SendDlgItemMessage( hdwnd, IDC_PROGRESS1, PBM_SETRANGE, 0, (LPARAM) MAKELPARAM (0, e_count));

               // TODO: Create a delete directory function
               DeleteFile( "C:\\Convert_Temp" );
               CreateDirectory( "C:\\Convert_Temp", NULL ); 
               for( i = 0; i < e_count; i++ ) {
                  offset = 0;
                  get_email_offset_and_size( im_header_filename, &offset, &size, i, e_count );
                  sprintf( eml_filename, "C:\\Convert_Temp\\email%d.eml", i );
                  extract_eml_files( im_database_filename, eml_filename, offset, size );
                  ZeroMemory( export_directory, sizeof( export_directory ) );
                  strncpy( export_directory, im_header_filename, strlen( im_database_filename ) - 4 );
                  strcat( export_directory, "\\" );
                  sprintf( new_eml_filename, "email%d.eml", i );
                  strcat( export_directory, new_eml_filename );
                  insert_attachments( eml_filename, im_attachments_directory, export_directory );
                  sprintf( debug_str, "%d of %d",i+1 ,e_count );
                  SetDlgItemText( hdwnd, IDC_XOFX, debug_str );
                  SendDlgItemMessage( hdwnd, IDC_PROGRESS1, PBM_STEPIT, 0, 0 );
                  SendMessage( hdwnd, WM_PAINT, 0, 0 );
               }
               sprintf( debug_str, "%d of %d DONE!",i ,e_count );
               SetDlgItemText( hdwnd, IDC_XOFX, debug_str );
            return 1;

            case IDCANCEL:
               // TODO: Create a delete directory function
               DeleteFile( "C:\\Convert_Temp" );
               EndDialog( hdwnd, 0 );
               PostQuitMessage( 0 );
            return 1;

            case IDC_ABOUT:
               DialogBox( hThisInst, MAKEINTRESOURCE(IDD_ABOUT), NULL, (DLGPROC) About_Box );
            return 1;
         }
   }

   return 0;
}

// TODO: FIX THIS
BOOL CALLBACK About_Box( HWND hdwnd, UINT message, WPARAM wparam, LPARAM lparam ) {
LoadLibrary(TEXT("Riched20.dll"));
SetDlgItemText( hdwnd, IDC_RICHEDIT26, "{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033\\deflangfe1033{\\fonttbl{\\f0\\fswiss\\fprq2\\fcharset0 Arial;}}{\\colortbl ;\\red0\\green0\\blue255;}\\viewkind4\\uc1\\pard\\f0\\fs20{\\field{\\*\\fldinst{HYPERLINK ""mailto:ReynardWare@gmail.com""}}{\\fldrslt{\\cf1\\ul ReynardWare@gmail.com}}}\\cf0\\ulnone\\f0\\fs20\\par}" );

   switch( message ) {
      case WM_COMMAND:
         switch(LOWORD(wparam)) {
            case IDOK:
               EndDialog( hdwnd, 0 );
            return 1;
         }
   }

   return 0;
}
