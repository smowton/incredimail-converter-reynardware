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

#ifndef __ABOUT_DLG__
#define __ABOUT_DLG__

// VERSION*****************************************
#define INCREDIMAIL_CONVERTER_VERSION "v0.53"
//*************************************************

BOOL CALLBACK About_Box( HWND, UINT, WPARAM, LPARAM );
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

void set_static_text( char *text, DWORD id, BOOL underline );
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

BOOL in_range( DWORD id, LPARAM lparam );
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

#endif