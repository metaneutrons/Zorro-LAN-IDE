/*
  hw.c 

  (C) 2018 Henryk Richter <henryk.richter@gmx.net>

  SDNet hardware interface to device


*/
#include <exec/libraries.h>
#include <proto/exec.h>
#include "device.h"
#include "hw.h"

#include <proto/vampire.h>
#include <vampire/vampire.h>
#include "vampuuid.h"
#include "enc28j60.h"

#ifndef PROTO_V2EXPNET
#define PORTID V_SDPORT
#else
#define PORTID V_WIFIPORT
#endif

extern const BYTE DeviceName[];

/* use generic board fields in current unit */
#define hw_allocated du_hwl0

#define HW_INTERVALDEF 10000
#define HW_DEFSPI 2


/* these calls might not run within the context of a process and
   will be called from varying tasks and without prior init:
    hw_Find_Boards()
    hw_AllocBoard()
    hw_ReleaseBoard()
    hw_GetMacAddress()

  these calls are in the context of the server task:
    hw_Setup()		- general init (board allocated already)
    hw_Shutdown()	- general shutdown (board still allocated)
    hw_Attach()		- make hardware ready for action (online)
    hw_Detach()		- put hardware into sleep mode (offline)

    hw_recv_sigmask()   - get signals for hardware (if any)
    hw_recv_pending()   - check for pending receive frames

    hw_recv_frame()	- receive one frame
    hw_send_frame()	- send one frame
    hw_check_link_change() - check if HW link parameters need update
                             and apply them (called only when no RX/TX
			     in progress)

    hw_config_init()    - set defaults for HW
    hw_config_update()  - update config of HW
*/

const BYTE vres_name[] = V_VAMPIRENAME;

/* return number of boards present in system */
ASM SAVEDS LONG hw_Find_Boards( ASMR(a0) DEVBASEP                  ASMREG(a0) )
{
 LONG ret = 0;
 struct Library *VampireBase;

 if( (VampireBase = OpenResource( (BYTE*)vres_name )) )
 {
	/* TODO: check for expansion port, i.e. distinguish V500/V600 */
	/* TODO: actually verify that we have a module here */
	ret = 1;
 }

 return ret;
}


ASM SAVEDS LONG hw_AllocBoard( ASMR(a0) DEVBASEP                  ASMREG(a0),
                               ASMR(d0) ULONG unit                ASMREG(d0) )
{
 LONG ret = 0;
 struct Library *VampireBase;

 if( (VampireBase = OpenResource( (BYTE*)vres_name )) ) /* OpenResource is probably redundant */
 {
  if( unit == 0 )
  {
  	ret = 1; /* def: looks good */
  	if( db->db_Units[unit].hw_allocated == 0 )
	{
		if( V_AllocExpansionPort( PORTID, (BYTE*)DeviceName ) )
 			ret = 0;
	}
	if( ret )
		db->db_Units[unit].hw_allocated++;
  }
 }

 return ret;
}


ASM SAVEDS LONG hw_ReleaseBoard( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                 ASMR(d0) ULONG unit                ASMREG(d0) )
{
 LONG ret = 0;
 struct Library *VampireBase;

 if( (VampireBase = OpenResource( (BYTE*)vres_name )) ) /* OpenResource is probably redundant */
 {
  if( unit == 0) /* only one device supported */
  {
  	ret = 1; /* def: looks good */
	if( db->db_Units[unit].hw_allocated > 0 )
	{
	 	db->db_Units[unit].hw_allocated--;
  	 	if( !db->db_Units[unit].hw_allocated )
	 	{
			V_FreeExpansionPort( PORTID );
		}
	}
  }
 }

 return ret;
}

/* store MAC address at "mac", 6 Bytes */
ASM SAVEDS LONG hw_GetMacAddress( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                  ASMR(d0) ULONG unit                ASMREG(d0),
				  ASMR(a1) UBYTE *mac                ASMREG(a1)
				)
{
  static UBYTE addr[6] = { 0x02,0x80,0x10,0x0b,0x0a,0xff};
  UBYTE vsn[8];
  LONG i;

  if( vampire_UUID( vsn ) > 0 )
  {
	for(i=0 ; i<3 ; i++ )
	{
		addr[3+i] = vsn[i] ^ vsn[i+5];
	}
  }
  for( i=0 ; i<6 ; i++ )
  	mac[i] = addr[i];

  return 0;
}

/* prepare all boards such that a hw_attach() will succeed */
ASM SAVEDS LONG hw_Setup( ASMR(a0) DEVBASEP                  ASMREG(a0) )
{
	LONG ret = 0;
	struct HWData *hwd = &db->db_hwdat;

	hwd->IntSig = -1;

	if( InitIntervalTimer( &hwd->ivtimer ) > 0 )
	{
		hwd->SigMask = hwd->ivtimer.IVT_SigMask;

		ret = 1;
	}

	return ret;
}

/* free resources, server is quitting */
ASM SAVEDS void hw_Shutdown(  ASMR(a0) DEVBASEP                  ASMREG(a0) )
{
	struct HWData *hwd = &db->db_hwdat;

	if( hwd->IntSig >= 0 )
		FreeSignal( hwd->IntSig );

	ExitIntervalTimer( &hwd->ivtimer );
}

/* make hardware ready for action (online) */
ASM SAVEDS LONG hw_Attach( ASMR(a0) DEVBASEP                  ASMREG(a0), 
                           ASMR(d0) ULONG unit                ASMREG(d0))
{
	struct HWData *hwd = &db->db_hwdat;
	LONG ret = 1;
	u08  flags;

	if( unit > 0 )
		return 0;

	flags = PIO_INIT_BROAD_CAST;
	if( hwd->multicast )
	     flags |= PIO_INIT_MULTI_CAST;

	if( hwd->fullduplex )
	    flags |= PIO_INIT_FULL_DUPLEX;

	if( db->db_Units[0].du_Flags & DUF_PROMISC )
	    flags |= PIO_INIT_PROMISC;

	enc28j60_SetSPISpeed( hwd->spispeed );

	if( enc28j60_init( db->db_Units[0].du_CFGAddr, flags ) != PIO_OK )
		ret = 0;
	else
	{
		enc28j60_SetSPISpeed( hwd->spispeed );
		StartIntervalTimer( &hwd->ivtimer, hwd->timervalue );
	}

	return ret;
}

/* put hardware into sleep mode (offline) */
ASM SAVEDS void hw_Detach( ASMR(a0) DEVBASEP                  ASMREG(a0), 
                           ASMR(d0) ULONG unit                ASMREG(d0))
{
	if( !unit )
	{
		struct HWData *hwd = &db->db_hwdat;
		StopIntervalTimer( &hwd->ivtimer );
		enc28j60_exit();
	}
}



ASM SAVEDS void hw_ConfigInit(   ASMR(a0) DEVBASEP                  ASMREG(a0) )
{
	struct HWData *hwd = &db->db_hwdat;

	hwd->timervalue = HW_INTERVALDEF;
	hwd->fullduplex = 0;
	hwd->spispeed   = HW_DEFSPI; /* default (see above) */
        hwd->multicast  = 0;

#if 0
	USHORT i;
	for( i=0 ; i < (USHORT)db->db_NBoards ; i++ )
	{	/* no need, these are defaults */
		db->db_Units[i].du_MTU = 1500;
		db->db_Units[i].du_BitPerSec = 10000000;

	}
#endif

}

ASM SAVEDS void hw_ConfigUpdate( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                 ASMR(a1) void*argsv                ASMREG(a1))
{
	struct HWConfig *args = (struct HWConfig *)argsv; /* see hwprivate.h */
	struct HWData *hwd = &db->db_hwdat;

	if( args->timervalue )
		hwd->timervalue = *args->timervalue;
	if( args->fullduplex )
		hwd->fullduplex = 1;
	if( args->spispeed )
		hwd->spispeed = *args->spispeed;
	if( args->multicast )
		hwd->multicast = 1;
}


ASM SAVEDS LONG hw_send_frame( ASMR(a0) DEVBASEP                  ASMREG(a0),
                               ASMR(d0) ULONG  unit               ASMREG(d0),
			       ASMR(a1) UBYTE *frame              ASMREG(a1),
                               ASMR(d1) ULONG  framesize          ASMREG(d1) )
{
   enc28j60_send( frame, framesize );
   return 1;
}


ASM SAVEDS LONG hw_recv_pending( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                 ASMR(d0) ULONG unit                ASMREG(d0) )
{
	return (LONG)enc28j60_has_recv();
}


ASM SAVEDS LONG hw_recv_frame( ASMR(a0) DEVBASEP                  ASMREG(a0),
                               ASMR(d0) ULONG unit                ASMREG(d0),
			       ASMR(a1) UBYTE *frame              ASMREG(a1))
{
   volatile SHORT sz;
   LONG ret;

   /* important: don't call this without enc28j60_has_recv() > 0 check */
   if( PIO_OK != enc28j60_recv( frame, 1518, (short*)&sz ) )
        ret = 0;
   else 
   	ret = (LONG)sz;

   return (LONG)ret;
}


ASM SAVEDS ULONG hw_recv_sigmask( ASMR(a0) DEVBASEP                  ASMREG(a0) )
{
	struct HWData *hwd = &db->db_hwdat;

	return hwd->SigMask;
}


/* check if HW link parameters need update */
ASM SAVEDS LONG hw_check_link_change( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(d0) ULONG unit                ASMREG(d0) )
{
	return 0;
}


/* process the list of multicast addresses for an appropriate
   filtering list on the hardware

   the server preprocesses active multicast listeners such that
   this call can decide whether to enable multicast altogether,
   filter with hashes etc.
*/
ASM SAVEDS LONG hw_change_multicast(  ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(a0) ULONG unit                ASMREG(d0),
                                      ASMR(a1) struct List *mcastlist    ASMREG(a1) )
{
	u08 flags;

	flags = PIO_INIT_BROAD_CAST;

	if( db->db_Units[0].du_Flags & DUF_PROMISC )
	    flags |= PIO_INIT_PROMISC;

	/* list non-empty ? (sufficient for ENC28J60 -> it doesn`t have hashing) */
	if( mcastlist->lh_Head->ln_Succ )
	     flags |= PIO_INIT_MULTI_CAST;

	enc28j60_broadcast_multicast_filter( flags );/* flags & (PIO_INIT_BROAD_CAST|PIO_INIT_MULTI_CAST) ); */

	return 1;
}
