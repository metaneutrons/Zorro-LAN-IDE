/*
  hwprivate.h

  (C) 2018 Henryk Richter <henryk.richter@gmx.net>

  private structures for hardware instance


*/
#ifndef _INC_HWPRIVATE_H
#define _INC_HWPRIVATE_H

#include "compiler.h"
#include <libraries/configvars.h>

/* sdnet/v2expnet specific: polling timer */
#include "intervaltimer.h"

#define HW_VENDOR  "a1k.org Matze Buggs Scrat"
#define HW_PRODUCT "Zorro LAN IDE CP"

/* this struct is available in devicebase and should carry global information */
struct HWData {
	struct Interrupt hwd_Interrupt; /* keep the interrupt at top of HWData !! */
	struct SignalSemaphore hwd_Sem;      /* list locking (global per unit) */
	APTR	hwd_act_boards[MAX_UNITS+1+3]; /* poll all active boards by ptr in interrupt, NULL terminated, followed by SigTask,Sigbit,ExecBase */

	ULONG	hwd_SigMask;	/* signal mask to wait for in server main instance */

	struct  IVT_TimerStruct ivtimer; /* Timer interval handling (via Interrupt) */
	LONG	TimerUsed;	/* number of instances running */
	LONG	hwd_IntSig;

	/* config options */
	ULONG	timervalue;	/* polling interval in ms */
	ULONG	fullduplex;	/* force full duplex */
	ULONG	multicast;	/* multicast recv enable */
	ULONG	flowcontrol;	/* enable RX flow control */
};

#define HW_CONFIGFILE "ENV:SANA2/enc624j6net.config"

/* appended to COMMON_TEMPLATE */
#define HW_CONFIGTEMPLATE "TIMER/K/N,FULLDUPLEX/S,MULTICAST/S,FLOWCONTROL/S"

/* referenced by server.c, hw.c */
struct HWConfig
{
   struct CommonConfig common;	/* import from hw.h, expected in server.c */
   ULONG *timervalue;
   ULONG fullduplex;
   ULONG multicast;
   ULONG flowcontrol;
};

#endif /* _INC_HWPRIVATE_H */
