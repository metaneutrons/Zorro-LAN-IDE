/*
  hw.h

  (C) 2018 Henryk Richter <henryk.richter@gmx.net>

  Hardware dependent functions


*/
#ifndef _INC_HW_H
#define _INC_HW_H

#include "compiler.h"

#ifndef DEVBASETYPE
struct devbase;
#define DEVBASETYPE struct devbase
#endif
#ifndef DEVBASEP
#define DEVBASEP DEVBASETYPE *db
#endif


/* find number of boards (stateless) */
ASM SAVEDS LONG hw_Find_Boards( ASMR(a0) DEVBASEP                  ASMREG(a0) );

/* allocate / release a board 
   note: alloc may be called more than once, keep track of number of openings
*/
ASM SAVEDS LONG hw_AllocBoard(   ASMR(a0) DEVBASEP                  ASMREG(a0),
                                 ASMR(d0) ULONG unit                ASMREG(d0) );
ASM SAVEDS LONG hw_ReleaseBoard( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                 ASMR(d0) ULONG unit                ASMREG(d0) );

ASM SAVEDS LONG hw_GetMacAddress(ASMR(a0) DEVBASEP                  ASMREG(a0),
                                 ASMR(d0) ULONG unit                ASMREG(d0), 
				 ASMR(a1) UBYTE *mac                ASMREG(a1) );

/* initialize global management stuff for hardware (called once on server startup) */
/*
   Call order
    - hw_Setup
 	     hw_ConfigInit
	     hw_ConfigUpdate
	     hw_Attach
	     	main loop: send/recv
	     hw_Detach
    - hw_Shutdown
*/
ASM SAVEDS LONG hw_Setup(    ASMR(a0) DEVBASEP                  ASMREG(a0) );
ASM SAVEDS void hw_Shutdown( ASMR(a0) DEVBASEP                  ASMREG(a0) );

ASM SAVEDS LONG hw_Attach( ASMR(a0) DEVBASEP                  ASMREG(a0), 
                           ASMR(d0) ULONG unit                ASMREG(d0));
ASM SAVEDS void hw_Detach( ASMR(a0) DEVBASEP                  ASMREG(a0), 
                           ASMR(d0) ULONG unit                ASMREG(d0));

ASM SAVEDS void hw_ConfigInit(   ASMR(a0) DEVBASEP                  ASMREG(a0) );
ASM SAVEDS void hw_ConfigUpdate( ASMR(a0) DEVBASEP                  ASMREG(a0), 
                                 ASMR(a1) void *                    ASMREG(a1) );

ASM SAVEDS LONG hw_send_frame( ASMR(a0) DEVBASEP                  ASMREG(a0),
                               ASMR(d0) ULONG  unit               ASMREG(d0),
			       ASMR(a1) UBYTE *frame              ASMREG(a1),
                               ASMR(d1) ULONG  framesize          ASMREG(d1) );

ASM SAVEDS LONG hw_recv_frame( ASMR(a0) DEVBASEP                  ASMREG(a0),
                               ASMR(d0) ULONG unit                ASMREG(d0),
			       ASMR(a1) UBYTE *frame              ASMREG(a1) );

ASM SAVEDS LONG hw_recv_pending( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                 ASMR(d0) ULONG unit                ASMREG(d0) );

ASM SAVEDS LONG hw_check_link_change( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(d0) ULONG unit                ASMREG(d0) );

ASM SAVEDS LONG hw_change_multicast(  ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(a0) ULONG unit                ASMREG(d0),
                                      ASMR(a1) struct List *             ASMREG(a1) );


/* kept this global for now: Z-II/Z-III boards share the same interrupt */
ASM SAVEDS ULONG hw_recv_sigmask( ASMR(a0) DEVBASEP                  ASMREG(a0) );



/* Configuration template: straight replica of Plipbox general options */
struct CommonConfig {
	ULONG  nospecialstats;
	LONG  *priority;
	ULONG *bps;
	ULONG *mtu;
};

#define COMMON_TEMPLATE "NOSPECIALSTATS/S,PRIORITY=PRI/K/N,BPS/K/N,MTU/K/N,"

/* additional definitions */
#include "hwprivate.h"


#endif /* _INC_HW_H */
