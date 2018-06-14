/*
  hwprivate.h

  (C) 2018 Henryk Richter <henryk.richter@gmx.net>

  private structures for hardware instance


*/
#ifndef _INC_HWPRIVATE_H
#define _INC_HWPRIVATE_H

#include "compiler.h"

/* sdnet/v2expnet specific: polling timer */
#include "intervaltimer.h"

/* this struct is available in devicebase and should carry global information */
struct HWData {
	ULONG	SigMask;	/* signal mask to wait for in server main instance */

	struct  IVT_TimerStruct ivtimer; /* Timer interval handling (via Interrupt) */
	LONG	IntSig;

	/* config options */
	ULONG	timervalue;	/* polling interval in ms */
	ULONG	fullduplex;	/* force full duplex */
	ULONG	spispeed;	/* SPI clock divider (>=1) */
	ULONG	multicast;	/* multicast recv enable */
};

/* referenced from server.c */
#if (defined PROTO_V2EXPNET)
#define HW_CONFIGFILE "ENV:SANA2/v2expeth.config"
#else
#define HW_CONFIGFILE "ENV:SANA2/sdnet.config"
#endif

/* appended to COMMON_TEMPLATE */
#define HW_CONFIGTEMPLATE "TIMER/K/N,FULLDUPLEX/S,SPISPEED/K/N,MULTICAST/S"

/* referenced by server.c, hw.c */
struct HWConfig
{
   struct CommonConfig common;	/* import from hw.h, expected in server.c */
   ULONG *timervalue;
   ULONG fullduplex;
   ULONG *spispeed;
   ULONG multicast;
};

#endif /* _INC_HWPRIVATE_H */
