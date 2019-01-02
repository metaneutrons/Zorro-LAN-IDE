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

#define HW_VENDOR  "Henryk Richter"

/* this struct is available in devicebase and should carry global information */
struct HWData {
	struct Interrupt hwd_Interrupt; /* keep the interrupt at top of HWData !! */
	APTR	hwd_act_boards[MAX_UNITS+1+2+1]; /* poll all active boards by ptr in interrupt, NULL terminated, followed by SigTask,Sigbit */
	/* contents of hwd_act_boards:
	    1 ULONG     processing ENABLE (1) or DISABLE (0)
	    n ULONG     BOARD LIST, NULL terminated
	    1 ULONG     SigTask
	    1 ULONG     SigBit
	*/

	struct  IVT_TimerStruct ivtimer; /* Timer interval handling (via Interrupt) */
	LONG	IntSig;
	ULONG	SigMask;	/* signal mask to wait for in server main instance */

	/* config options */
	ULONG	timervalue;	/* polling interval in ms */
	ULONG	fullduplex;	/* force full duplex */
	ULONG	spispeed;	/* SPI clock divider (>=1) */
	ULONG	multicast;	/* multicast recv enable */
	ULONG	interrupt;	/* enable interrupt */
};

/* referenced from server.c */
#if (defined PROTO_V2EXPNET)

#define HW_CONFIGFILE "ENV:SANA2/v2expeth.config"
/* appended to COMMON_TEMPLATE */
#define HW_CONFIGTEMPLATE "TIMER/K/N,FULLDUPLEX/S,SPISPEED/K/N,MULTICAST/S,INTERRUPT/S"
#define HW_PRODUCT "VampireV500V2 ENC28J60 ExpansionPort"

#else

#define HW_CONFIGFILE "ENV:SANA2/sdnet.config"
/* appended to COMMON_TEMPLATE */
#define HW_CONFIGTEMPLATE "TIMER/K/N,FULLDUPLEX/S,SPISPEED/K/N,MULTICAST/S"
#define HW_PRODUCT "Vampire SDNet ENC28J60"

/* appended to COMMON_TEMPLATE */
#endif


/* referenced by server.c, hw.c */
struct HWConfig
{
   struct CommonConfig common;	/* import from hw.h, expected in server.c */
   ULONG *timervalue;
   ULONG fullduplex;
   ULONG *spispeed;
   ULONG multicast;
   ULONG interrupt;
};

#endif /* _INC_HWPRIVATE_H */
