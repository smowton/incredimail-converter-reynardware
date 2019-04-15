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

#ifndef __BASE_64__
#define __BASE_64__

void encodeblock( unsigned char in[3], unsigned char out[4], int len );
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

void encode( FILE *infile, FILE *outfile, int linesize );
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