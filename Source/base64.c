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


void encode( FILE *infile, FILE *outfile, int linesize ) {

unsigned char in[3], out[4];
int len, blocksout = 0;
char new_line[] = {0x0D, 0x0A};

   while( (len = fread(in, 1, 3, infile)) >= 1 ) {
      encodeblock( in, out, len );
	  fwrite(out, 1, 4, outfile);
      blocksout++;
      if( blocksout >= (linesize/4) ) {
         if( blocksout ) {
			fwrite(new_line, 1, sizeof(new_line), outfile);
         }
         blocksout = 0;
      }
   }

   if (blocksout != 0) {
	   fwrite(new_line, 1, sizeof(new_line), outfile);
   }
}