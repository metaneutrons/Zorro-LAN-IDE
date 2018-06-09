/*
  vampuuid.h 

  author: Henryk Richter <henryk.richter@gmx.net>

  purpose: Vampire UUID reading prototypes

*/
#ifndef _INC_vampuuid_H
#define _INC_vampuuid_H

#include "compiler.h"

/*
  Purpose: Read UUID of Vampire cards 
  input:   buffer - storage for the returned UUID (8 Bytes)
  output:  success/failure
            >0 - all fine, ready to work
	   <=0 - init problem (codes undefined yet)
*/
ASM SAVEDS int vampire_UUID( ASMR(a1) unsigned char *buffer ASMREG(a1) );


#endif /* __INC_vampuuid_H */
