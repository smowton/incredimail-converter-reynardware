#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "base64.h"


void encodeblock( unsigned char in[3], unsigned char out[4], int len )  {

const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}


void encode( HANDLE infile, HANDLE outfile, int linesize ) {

unsigned char in[3], out[4];
int len, blocksout = 0;
int byteread, temp;
char new_line[] = {0x0D, 0x0A};

   byteread = 1;

   while( byteread != 0 ) {
      len = 0;
      memset( in, 0, sizeof( in ) );
      ReadFile( infile, in, 0x03, &byteread, NULL );
      len = byteread;
      if( len ) {
         encodeblock( in, out, len );
         WriteFile( outfile, out, 0x04, &temp, NULL );
         blocksout++;
      }
      if( blocksout >= (linesize/4) || byteread == 0 ) {
         if( blocksout ) {
            WriteFile( outfile, new_line, sizeof( new_line ), &temp, NULL );
         }
         blocksout = 0;
      }
   }
}