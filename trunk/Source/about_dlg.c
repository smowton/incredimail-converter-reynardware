#include <windows.h>
#include <windowsx.h>
#include <string.h>

#include "resource.h"
#include "about_dlg.h"

HWND g_hwnd;

BOOL CALLBACK About_Box( HWND hdwnd, UINT message, WPARAM wparam, LPARAM lparam ) {

static BOOL email_inrange;  // Is the mouse in range of the static email control box
static BOOL www_inrange;    // Is the mouse in range of the static www control box

   switch( message ) {
      case WM_PAINT:
         set_static_text( "Reynardware@gmail.com", IDC_EMAIL_LINK, email_inrange );
         set_static_text( "Project Website", IDC_WWW_LINK, www_inrange );
         return 0;  // WM_PAINT must return 0

      case WM_INITDIALOG:
         g_hwnd = hdwnd;          // set the global handle for support functions
         SetDlgItemText( g_hwnd, IDC_VERSION, INCREDIMAIL_CONVERTER_VERSION );
         return 1;
 
      case WM_MOUSEMOVE:
         email_inrange = in_range( IDC_EMAIL_LINK, lparam );
         www_inrange   = in_range( IDC_WWW_LINK, lparam );
         SendMessage( g_hwnd, WM_PAINT, 0, 0 );
         InvalidateRect( g_hwnd, NULL, FALSE);                  
         return 1;

      case WM_COMMAND:
         switch(LOWORD(wparam)) {
            case IDOK:
               EndDialog( hdwnd, 0 );
               return 1;

            case IDC_EMAIL_LINK:
               ShellExecute(NULL, TEXT("open"), "mailto:reynardware@gmail.com", NULL, NULL, SW_SHOWNORMAL);
               return 1;
            
            case IDC_WWW_LINK:
               ShellExecute(NULL, TEXT("open"), "http://code.google.com/p/incredimail-converter-reynardware/", NULL, NULL, SW_SHOWNORMAL);
               return 1;
         }
   }

   return 0;
}


void set_static_text( char *text, DWORD id, BOOL underline ) {

HWND static_control;
HDC static_dc;
PAINTSTRUCT ps;
DWORD dwStyle;
DWORD dwDTStyle;
LOGFONT lf;
HFONT hf, hfUnderlined;
HANDLE hOld;
RECT rect;

   // inital stuff
   static_control = GetDlgItem( g_hwnd, id );
   static_dc = BeginPaint( static_control, &ps );
   GetClientRect(static_control, &rect);

   // get and set the style
   dwStyle = GetWindowLongPtr(static_control, GWL_STYLE);
   if(dwStyle & SS_CENTER)      dwDTStyle |= DT_CENTER;
	if(dwStyle & SS_RIGHT)       dwDTStyle |= DT_RIGHT;
	if(dwStyle & SS_CENTERIMAGE) dwDTStyle |= DT_VCENTER;

   // underline the font
   hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
   GetObject(hf, sizeof lf, &lf);
   if( underline == TRUE ) {
      lf.lfUnderline = TRUE;
   } else {
      lf.lfUnderline = FALSE;
   }
   hfUnderlined = CreateFontIndirect(&lf);
   hOld = SelectObject(static_dc, hfUnderlined);

   // set the color of the text and background
   SetTextColor( static_dc, RGB(0, 0, 255) );  // blue
   SetBkColor(static_dc, RGB(236, 233, 216));
         
   // draw the text
   DrawText( static_dc, text, -1, &rect, dwDTStyle ); 

   // end it
   SelectObject(static_dc, hOld);
   EndPaint( static_control, &ps );
}


BOOL in_range( DWORD id, LPARAM lparam ) {

RECT rect, drect, qrect;
DWORD xpos, ypos;
HWND static_control;
BOOL inrange;

   static_control = GetDlgItem( g_hwnd, id );
   GetWindowRect(static_control, &rect);
   GetClientRect(static_control, &qrect);
   GetWindowRect(g_hwnd, &drect);

   xpos = GET_X_LPARAM(lparam); 
   ypos = GET_Y_LPARAM(lparam);

   if( xpos > (DWORD) ( (rect.left - drect.left) ) && xpos < (DWORD) ( (rect.left - drect.left) + qrect.right ) &&
       ypos >  (DWORD) (rect.top - drect.top - 35) && ypos < (DWORD) ( (rect.top - drect.top - 25) + qrect.bottom )) {
       inrange = TRUE;
   } else {
       inrange = FALSE;
   }

   return inrange;
}