/*
   Device Header

   (C) 2018 Henryk Richter <henryk.richter@gmx.net>

   taken from commandline (compiler options)
    -DDEVICENAME=blah.device
    -DDEVICEVERSION=45
    -DDEVICEREVISION=36
    -DDEVICEDATE=2.12.2012

*/

#include <exec/resident.h>
#include <exec/nodes.h>
#include <exec/initializers.h>
#include <exec/libraries.h>
#include "compiler.h"
#include "device.h"

#define xstr(a) str(a)
#define str(a) #a

#if  1

/* Enable this if you want pure C for the device. (disable compilation of romtag.asm in that case)
   I personally prefer the small ASM blob to steer away from linking challenges.
*/
const BYTE DeviceName[];
const BYTE DeviceVersionString[];
const APTR DeviceInitTab[];


const struct Resident RomTag = {
	RTC_MATCHWORD,
	( struct Resident* ) &RomTag,
	( struct Resident* ) &RomTag + 1,
	RTF_AUTOINIT,
	DEVICEVERSION,
	NT_DEVICE,
	0,
	(BYTE*)DeviceName,
	(BYTE*)DeviceVersionString+6,
	(APTR)DeviceInitTab
};
#endif

const BYTE DeviceVersionString[] = "$VER: " xstr(DEVICENAME) " " xstr(DEVICEVERSION) "." xstr(DEVICEREVISION) " (" xstr(DEVICEDATE) ")";
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


