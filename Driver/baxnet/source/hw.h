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

/* states for each NIC speed */
#define HW_MAC_ISACTIVE(_status_,_spd_) ( ((_status_)>>(_spd_)) & 3 ) /* returns 0 if inactive, 0...3 if active */
#define HW_MAC_OFF       0 /* this speed is inactive */
#define HW_MAC_ON        1 /* on, half duplex */ 
#define HW_MAC_ON_FDX    2 /* on, full duplex */
#define HW_MAC_ON_FDX_FC 3 /* on, full duplex, flow-control */

/*
  MAC definitions 
  (flag Mask)
  -> conceptually, this layout can be used for current
     running mode as well as autonegotiation capabilities 
     masks for local NIC and peer
  -> is fiber optic Ethernet relevant for us Amigans?
*/
#define HW_MAC_INVALID        (-1) /* no info available      */
#define HW_MAC_LINK_UP        1 /* link is up                */
#define HW_MAC_AUTONEGOTIATION 2 /* autonegotiation enabled   */

/* two bits each to handle the four different states per speed */
#define HW_MACB_SPEED10      2  /* bit for 10 MBit/s mask    */
#define HW_MACB_SPEED100     4  /* bit for 100 MBit/s mask   */
#define HW_MACB_SPEED100T2   6  /* bit for 100 MBit/s mask (100BaseT2) */
#define HW_MACB_SPEED100T4   8  /* bit for 100 MBit/s mask (100BaseT4) */
#define HW_MACB_SPEED1000    10 /* ... */
#define HW_MACB_SPEED2500    12
#define HW_MACB_SPEED5000    14
#define HW_MACB_SPEED10000   16 /* Am I optimistic ? */
#define HW_MACB_SPEEDS_END   18 /* this is the next free bit */
#define HW_MAC_SPEEDSMASK    ((1<<HW_MACB_SPEEDS_END)-4)
#define HW_MAC_NEXTSPEED(_a_) ( (_a_)+2 )





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

/*
  send a single frame to the hardware:
   - either fully assembled frame (14 bytes DMAC,SMAC,TYPE followed by payload
   - or payload in "frame" and 14 byte header in "header"
   - provide Makefile-level definition of HW_DMA_TX in the latter case

   - note: if you pass "header==NULL" with active HW_DMA_TX definition,
     the hw_send_frame assumes the header to be present (SANA-II RAW frames) and
     is handling "frame" as fully assembled frame
*/
ASM SAVEDS LONG hw_send_frame( ASMR(a0) DEVBASEP                  ASMREG(a0),
                               ASMR(d0) ULONG  unit               ASMREG(d0),
			       ASMR(a1) UBYTE *frame              ASMREG(a1),
                               ASMR(d1) ULONG  framesize          ASMREG(d1) 
#ifdef HW_DMA_TX
                              ,ASMR(a2) UBYTE *header             ASMREG(a2)
#endif
                             );

ASM SAVEDS LONG hw_recv_frame( ASMR(a0) DEVBASEP                  ASMREG(a0),
                               ASMR(d0) ULONG unit                ASMREG(d0),
			       ASMR(a1) UBYTE *frame              ASMREG(a1) );

ASM SAVEDS LONG hw_recv_pending( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                 ASMR(d0) ULONG unit                ASMREG(d0) );

ASM SAVEDS LONG hw_check_link_change( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(d0) ULONG unit                ASMREG(d0) );

ASM SAVEDS LONG hw_change_multicast(  ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(d0) ULONG unit                ASMREG(d0),
                                      ASMR(a1) struct List *             ASMREG(a1) );

ASM SAVEDS LONG hw_get_mac_status(    ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(d0) ULONG unit                ASMREG(d0) );

/* read PHY register (return -1 in case of error) */
ASM SAVEDS LONG hw_read_phy(          ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(d0) ULONG unit                ASMREG(d0),
				      ASMR(d1) ULONG reg                 ASMREG(d1) );

/* write PHY register (return -1 in case of error) */
ASM SAVEDS LONG hw_write_phy(         ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(d0) ULONG unit                ASMREG(d0),
				      ASMR(d1) ULONG reg                 ASMREG(d1),
				      ASMR(d2) ULONG value               ASMREG(d2) );


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
