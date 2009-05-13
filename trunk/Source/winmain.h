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

#ifndef __WINMAIN__
#define __WINMAIN__

BOOL CALLBACK Winload( HWND, UINT, WPARAM, LPARAM );
//***************************************************************************
// INPUTS:
//     HWND hdwnd - The handle of the Dialog Box Window
//     UNIT message - The dialog box's window messages
//     WPARM wparam - Wparam for the dialog box 
//     LPAMRAM lparam - Lparam for the dialog box 
//
// OUTPUTS:  None
//
// RETURN VALUE: BOOL - (1) Function success and (0) Failure
//
// DESCRIPTION:
//   This is the main function of the Dialog box.
//
//***************************************************************************


void WINAPI process_emails();
//***************************************************************************
// INPUTS: None
//
// OUTPUTS: None
//
// RETURN VALUE: Void
//
// DESCRIPTION:  This function processes all the emails within a dialog box
//
//***************************************************************************

#endif