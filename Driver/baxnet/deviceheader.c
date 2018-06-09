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
#include "compiler.h"
#include "device.h"

/* Enable this if you want pure C for the device. (disable compilation of romtag.asm in that case)
   I personally prefer the small ASM blob to steer away from linking challenges.
*/
#if  1

ASM LONG LibNull( void )
{
	return 0;
}

extern const BYTE DeviceName[];
extern const BYTE DeviceVersionString[];
extern const APTR DeviceInitTab[];

const struct Resident _00RomTag = {
	RTC_MATCHWORD,
	( struct Resident* ) &_00RomTag,
	( struct Resident* ) &_00RomTag + 1,
	RTF_AUTOINIT,
	DEVICEVERSION,
	NT_DEVICE,
	0,
	(BYTE*)DeviceName,
	(BYTE*)DeviceVersionString+6,
	(APTR)DeviceInitTab
};
#endif



