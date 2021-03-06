#ifndef SANA2_SANA2DEVQUERYEXT_H
#define SANA2_SANA2DEVQUERYEXT_H

/*
**	$VER: sana2devqueryext.h 0.12 (10-Jun-2018)
**	Definitions for SANA-II extended device query
**	(c) 2018, Timm S. Mueller and Olaf Barthel
**
**	Permission is hereby granted, free of charge, to any person obtaining
**	a copy of this file, without limitation the rights to copy, publish,
**	and distribute this file in unmodified form, and to use it for the
**	creation of software.
**	We strongly advise you not to release software into the public based on
**	modifications or extensions of the definitions below, without seeking
**	prior agreement with the authors regarding your changes.
**
**	Application protocol:
**	1. Using NSCMD_DEVICEQUERY, check for availability of S2_DEVICEQUERYEXT.
**	2. For each attribute provide a struct S2DevQueryExtParam. Fill in a
**	   pointer to your data, its length, and initialize Actual to 0.
**	3. For each attribute requested, supply a tagitem with a pointer to the
**	   respective S2DevQueryExtParam structure as its value. Pass the taglist
**	   in ios2_Data. After return from the device and no I/O error,
**	4. check for the expected data size using param.Actual == sizeof(value).
**	   Check for the presence of any returned data using param.Actual != 0.
**	   The length of strings include a terminating zero byte, so param.Actual
**	   for a string of length 0 would be 1.
**	5. To only retrieve the number of bytes that would be written, set
**	   param.Data to NULL.
**	6. Unless otherwise noted, numerical values are of type ULONG by default.
**
**	Driver protocol:
**	1. Support S2_DEVICEQUERYEXT per NSCMD_DEVICEQUERY.
**	2. Iterate supplied tags in ios2_Data. For each tag,
**	3. if param->Data is NULL, write the length that would be written
**	   into param->Actual. In the case of strings, always include the
**	   terminating zero byte in this length.
**	4. If param->Data is not NULL, check for
**	   param->Length >= sizeof(your_data). Copy your data into the
**	   buffer provided by param->Data. Under no circumstance may the length
**	   being written exceed param->Length. Write the length actually being
**	   written into param->Actual. At your discretion, shorten informative,
**	   non-indexing strings to fit into the user-supplied buffer -- but you
**	   are not allowed to write strings without a terminating zero byte,
**	   which must fit into the buffer as indicated by param->Length.
*/

#include <devices/sana2.h>

struct S2DevQueryExtParam
{
	APTR Data;      /* Ptr to data buffer you provide */
	LONG Length;    /* Length of data buffer you provide */
	LONG Actual;    /* Number of bytes actually written */
};

#define S2_DEVICEQUERYEXT               (S2_START + 0x100)
#define S2_DevQueryExtTagBase           (S2_Dummy + 0x100)

/* ptr to string containing the vendor name: */
#define S2_DevQueryExtVendorName        (S2_DevQueryExtTagBase + 1)
/* ptr to string containing the product name: */
#define S2_DevQueryExtProductName       (S2_DevQueryExtTagBase + 2)
/* Current physical link status, see definitions below: */
#define S2_DevQueryExtLinkStatus        (S2_DevQueryExtTagBase + 3)
/* Current physical link speed, see definitions below: */
#define S2_DevQueryExtLinkSpeed         (S2_DevQueryExtTagBase + 4)

/* proposal added by Henryk Richter: access to MII */
/* MII is the standardized ethernet interface to the 
   PHY registers. It allows to read and modify autonegotiation
   settings and status, as well as manual reconfiguration of
   link parameters. In order to ease the burden on the
   SANA-II driver, this interface is transparent in the sense
   that no processing of MII registers is necessary.

   Please note that MII read/write is slow. Use with discretion.

   The format of the Data APTR is per entry as follows:
    UWORD ID
    UWORD Content

   S2_DevQueryExtGetMII
    Please pass a writable Data array of Length=2*NumIDs to the call and
    initialize the IDs you are interested in. The driver will initialize
    the Content of the IDs it knows about. Should a requested ID be
    unsupported by the driver or underlying chipset, the driver will 
    overwrite the respective IDs with S2_DEVQUERYEXT_MII_INVALID and
    clear the associated Content field.
    In case of fatal error, the "Actual" field of S2DevQueryExtParam
    will be 0. Otherwise, the last successfully updated location + 2
    will be returned.

   S2_DevQueryExtSetMII
    Please pass a Data array of Length=2*NumIDs to the call and initialize
    the IDs to write along with their content. The driver will write the 
    MII registers corresponding with the IDs in the given order.
    In case of fatal error, the "Actual" field of S2DevQueryExtParam
    will be 0. Otherwise, the last successfully updated location + 2 
    will be returned.

*/
#define S2_DevQueryExtGetMII		(S2_DevQueryExtTagBase + 0x20)
#define S2_DevQueryExtSetMII		(S2_DevQueryExtTagBase + 0x21)

struct S2DevQueryMIIParam
{
	UWORD ID;
	UWORD Content;
};
#define S2_DEVQUERYEXT_MII_INVALID	0xfe


#define S2_DEVQUERYEXT_LINK_UNDEFINED   0xffffffff
#define S2_DEVQUERYEXT_LINK_DOWN        0
#define S2_DEVQUERYEXT_LINK_UP          1

#define S2_DEVQUERYEXT_SPEED_UNDEFINED  0xffffffff
#define S2_DEVQUERYEXT_SPEED_OTHER      0
#define S2_DEVQUERYEXT_SPEED_10M        1
#define S2_DEVQUERYEXT_SPEED_100M       2
#define S2_DEVQUERYEXT_SPEED_1000M      3
#define S2_DEVQUERYEXT_SPEED_10000M     4

#endif
