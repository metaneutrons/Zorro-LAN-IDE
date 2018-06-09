/*
  devinit.c 

  (C) 2018 Henryk Richter <henryk.richter@gmx.net>

  initialization structures and data

*/
#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <dos/dostags.h>
#include <utility/tagitem.h>
#include <exec/lists.h>
#include "device.h"
#include "hw.h"
#include "macros.h"
#include <exec/initializers.h>

#define xstr(a) str(a)
#define str(a) #a

#ifndef DEVICEEXTRA
#define DEVICEEXTRA
#endif

const BYTE DeviceVersionString[] = "$VER: " xstr(DEVICENAME) " " xstr(DEVICEVERSION) "." xstr(DEVICEREVISION) xstr(DEVICEEXTRA) " (" xstr(DEVICEDATE) ")";
const BYTE DeviceName[] = xstr(DEVICENAME);


const APTR DeviceFunctions[] = {
	(APTR) DevOpen,
	(APTR) DevClose,
	(APTR) DevExpunge,
	(APTR) LibNull,
	(APTR) DevBeginIO,
	(APTR) DevAbortIO,
	(APTR) -1
};


#define WORDINIT(_a_) UWORD _a_ ##W1; UWORD _a_ ##W2; UWORD _a_ ##ARG;
#define LONGINIT(_a_) UBYTE _a_ ##A1; UBYTE _a_ ##A2; ULONG _a_ ##ARG;
struct DeviceInitData
{
	WORDINIT(w1) 
	LONGINIT(l1)
	WORDINIT(w2) 
	WORDINIT(w3) 
	WORDINIT(w4) 
	LONGINIT(l2)
	ULONG end_initlist;
} DeviceInitializers =
{
	INITBYTE(     STRUCTOFFSET( Node,  ln_Type),      NT_DEVICE),
	0x80, (UBYTE) STRUCTOFFSET( Node,  ln_Name),      (ULONG) &DeviceName[0],
	INITBYTE(     STRUCTOFFSET(Library,lib_Flags),    LIBF_SUMUSED|LIBF_CHANGED ),
	INITWORD(     STRUCTOFFSET(Library,lib_Version),  DEVICEVERSION  ),
	INITWORD(     STRUCTOFFSET(Library,lib_Revision), DEVICEREVISION ),
	0x80, (UBYTE) STRUCTOFFSET(Library,lib_IdString), (ULONG) &DeviceVersionString[6],
	(ULONG) 0
};


const APTR DeviceInitTab[] = {
	(APTR) sizeof( DEVBASETYPE ),
	(APTR) &DeviceFunctions,
	(APTR) &DeviceInitializers,
	(APTR) DevInit
};


