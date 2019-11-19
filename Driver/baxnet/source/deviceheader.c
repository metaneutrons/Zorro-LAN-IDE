/*
   Device Header - ROMTAG

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
#ifdef HAVE_VERSION_H
#include "version.h"
#endif
#include "compiler.h"
#include "device.h"

/* Enable this if you want pure C for the device. (disable compilation of romtag.asm in that case)
   I personally prefer a small ASM blob to steer away from linking challenges.
*/
#if  1
extern const char DeviceName[];
extern const char DeviceVersionString[];
extern const APTR DeviceInitTab[];

/* force moveq #0,d0;rts in front of the ROMTAG */
#if 1
struct myrt0 {
 ULONG ret0;
 struct Resident rt;
};

/* There is a trick here:
    LibNull() is actually a function returning 0
*/
const struct myrt0 LibNull =
{
 0x70004E75,
 {
	RTC_MATCHWORD,
	( struct Resident* ) ((char *)&LibNull+sizeof(ULONG)),
	( struct Resident* ) ((char *)&LibNull+sizeof(ULONG)+sizeof(struct Resident)),
	RTF_AUTOINIT,
	DEVICEVERSION,
	NT_DEVICE,
	0,
	(char*)DeviceName,
	(char*)DeviceVersionString+6,
	(APTR)DeviceInitTab
 }
};
#else
/* hope that the compiler will put LibNull first */
ASM LONG LibNull( void )
{
	return 0;
}
static const struct Resident _00RomTag = {
	RTC_MATCHWORD,
	( struct Resident* ) &_00RomTag,
	( struct Resident* ) &_00RomTag + 1,
	RTF_AUTOINIT,
	DEVICEVERSION,
	NT_DEVICE,
	0,
	(char*)DeviceName,
	(char*)DeviceVersionString+6,
	(APTR)DeviceInitTab
};
#endif

#endif



