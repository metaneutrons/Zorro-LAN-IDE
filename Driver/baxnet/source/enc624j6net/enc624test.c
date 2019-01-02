/*
  enc624test.c 

  (C) 2018 Henryk Richter <henryk.richter@gmx.net>

  Matze-Scrat-Buggs ZII card hardware test


*/
#include <exec/libraries.h>
#include <proto/exec.h>

#define ENC_MANUFACTURER 2588
#define ENC_BOARD 123

#include <proto/expansion.h>
#include <proto/dos.h>
#include <libraries/configvars.h>
#include <hardware/intbits.h>

#if 0
#include "device.h"
#include "hw.h"

#include "enc624j6l.h"
#endif
#include "registers.h"


#define READREG(_x_,_off_)      *( (volatile USHORT*)( (volatile UBYTE*)(_x_) + (_off_)) )
#define WRITEREG(_x_,_off_,_y_) *( (volatile USHORT*)( (volatile UBYTE*)(_x_) + (_off_)) )    = (_y_);
#define SETREG(_x_,_off_,_y_)   *( (volatile USHORT*)( (volatile UBYTE*)(_x_) + (_off_) + SET_OFFSET )) = (_y_);
#define CLRREG(_x_,_off_,_y_)   *( (volatile USHORT*)( (volatile UBYTE*)(_x_) + (_off_) + CLR_OFFSET )) = (_y_);

const BYTE exp_name[] = "expansion.library";

int main( int argc, char **argv )
{
 LONG ret = 0;
 ULONG ul;
 volatile APTR board = (0);
 volatile UBYTE *uboard;
 struct Library *ExpansionBase;
 struct ConfigDev *cfg = (0);

 ExpansionBase = OpenLibrary( (BYTE*)exp_name, 37 );
 if( !ExpansionBase )
 {
 	Printf("Cannot open Expansion. quitting\n");
 	goto quit;
 }

 cfg = FindConfigDev( cfg, ENC_MANUFACTURER, ENC_BOARD );
 if( cfg )
 {
 	board = (volatile APTR)cfg->cd_BoardAddr;
 }
 if( !board )
 {
 	Printf("Board %ld / %ld not found in expansions\n",(ULONG)ENC_MANUFACTURER,(ULONG)ENC_BOARD);
 	goto quit;
 }
 Printf("Board found at 0x%lx\n",(ULONG)board);

 /* 16 Bit R/W Test */
 Printf("16 Bit reg test\n",(ULONG)board);
 WRITEREG( board, EUDAST, 0x1234 );
 ul = READREG( board, EUDAST );
 if( ul != 0x1234 )
 {
 	Printf("Board error: wrote 0x1234, got 0x%lx\n",ul);
 	ret = 1;
 }
 WRITEREG( board, EUDAST, 0x0000 );

 /* Byte R/W tests */
 Printf("8 Bit buffer tests\n",(ULONG)board);

 WRITEREG( board, 0xBFE, 0x0000 );
 uboard = (volatile UBYTE*)board;

 *(uboard+0xBFE) = 0xda;
 ul = READREG( board, 0xBFE );
 if( ul != 0xda00 )
 {
 	Printf("Board error: expected 0xDA00, got 0x%lx on upper byte test\n",ul);
 	ret = 1;
 }

 WRITEREG( board, 0xBFE, 0x0000 );
 *(uboard+0xBFF) = 0xbf;
 ul = READREG( board, 0xBFE );
 if( ul != 0x00bf )
 {
 	Printf("Board error: expected 0x00BF, got 0x%lx on lower byte test\n",ul);
 	ret = 1;
 }

 if( ret )
  Printf("Error(s) encountered\n");
 else
  Printf("Board looking good as far as tested\n");

quit:
 if( ExpansionBase )
	 CloseLibrary(ExpansionBase);

 return ret;
}

