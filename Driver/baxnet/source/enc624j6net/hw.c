/*
  hw.c 

  (C) 2018 Henryk Richter <henryk.richter@gmx.net>

  Matze-Scrat-Buggs ZII card hardware interface to device


*/
#include <exec/libraries.h>
#include <proto/exec.h>

/*#include <clib/expansion_protos.h>
#include <pragmas/expansion_pragmas.h>*/
#include <proto/expansion.h>
#include <libraries/configvars.h>
#include <hardware/intbits.h>

#include "device.h"
#include "hw.h"

#include "enc624j6l.h"
#include "registers.h"

extern const BYTE DeviceName[];

#define ENC_MANUFACTURER 2588
#define ENC_BOARD 123

/* use generic board fields in current unit */
#define duh_online    du_hwl0
#define duh_BASE      du_hwp0
#define duh_RefCount  du_hwl1

#define HW_INTERVALDEF 100000L

/* this is a BIG TODO! Make plipbox source multi-board aware */
#define  BOARD hwb->hwb_boards[0]

/* interrupt (choice here is INTB_EXTER or INTB_PORTS, depending on solder blob */
/*#define  HW_INTSOURCE INTB_EXTER */ /* Int 6 */
#define  HW_INTSOURCE INTB_PORTS /* Int 2 */

#define READREG(_x_,_off_)      *( (volatile USHORT*)( (volatile UBYTE*)(_x_) + (_off_)) )
#define WRITEREG(_x_,_off_,_y_) *( (volatile USHORT*)( (volatile UBYTE*)(_x_) + (_off_)) )    = (_y_);
#define SETREG(_x_,_off_,_y_)   *( (volatile USHORT*)( (volatile UBYTE*)(_x_) + (_off_) + SET_OFFSET )) = (_y_);
#define CLRREG(_x_,_off_,_y_)   *( (volatile USHORT*)( (volatile UBYTE*)(_x_) + (_off_) + CLR_OFFSET )) = (_y_);

void myhw_ControlInterrupts( DEVBASEP );
void myhw_ControlIntervalTimer( DEVBASEP );


/* these calls might not run within the context of a process and
   will be called from varying tasks and without prior init:
    hw_Find_Boards()	- this one just identifies available HW and returns #boards
    hw_AllocBoard()
    hw_ReleaseBoard()
    hw_GetMacAddress()  - get default MAC address from board

  these calls are in the context of the server task:
    hw_Setup()	        - general init (board allocated already)
    hw_Shutdown()       - general shutdown (board still allocated)
    hw_Attach()	        - make hardware ready for action (online)
    hw_Detach()	        - put hardware into sleep mode (offline)

    hw_recv_sigmask()   - get signals for hardware (if any, i.e. signal on interrupt)
    hw_recv_pending()   - check for pending receive frames

    hw_recv_frame()	- receive one frame
    hw_send_frame()	- send one frame
    hw_check_link_change() - check if HW link parameters need update
                             and apply them (called only when no RX/TX
			     in progress)

    hw_ConfigInit()     - set defaults for HW
    hw_ConfigUpdate()   - update config of HW

    hw_change_multicast - re-initialize multicast filter for new address list
    hw_get_mac_status()	- running parameters (optional), S2_DEVICEQUERYEXT
*/

const BYTE exp_name[] = "expansion.library";
const BYTE intname[] = "enc264j6net SanaII driver";

/* return number of boards present in system */
ASM SAVEDS LONG hw_Find_Boards( ASMR(a0) DEVBASEP                  ASMREG(a0) )
{
 LONG i,ret = 0;
 struct Library *ExpansionBase;
 struct ConfigDev *cfg = (0);
 APTR boardbase;

 if( (ExpansionBase = OpenLibrary( (BYTE*)exp_name, 37 )) )
 {
	/* TODO: actually verify that we have a module here */
	/* ret = 1; */

	for( i=0 ; i < MAX_UNITS ; i++ )
	{
		cfg = FindConfigDev( cfg, ENC_MANUFACTURER, ENC_BOARD );
		if( !cfg )
			break;
		boardbase = (APTR)cfg->cd_BoardAddr;
		db->db_Units[i].duh_BASE = boardbase;
		D(("Board at %lx\n",(ULONG)cfg->cd_BoardAddr ));

		/* bring board to sane state (i.e. perform soft reset) */
		/* if( boardbase < (APTR)0x40000000 ) */ /* debug only */
		enc624j6l_CheckBoard( boardbase );
	}
	ret = i;
	D(("%ld boards found\n",ret));
	CloseLibrary( ExpansionBase );
 }

 return ret;
}


ASM SAVEDS LONG hw_AllocBoard( ASMR(a0) DEVBASEP                  ASMREG(a0),
                               ASMR(d0) ULONG unit                ASMREG(d0) )
{
#if 0
	LONG i;
	struct ConfigDev *cfg = (0);

	if( unit > db->db_NBoards )
		return 0;

	for( i=0 ; i < db->db_NBoards ; i++ )
	{
		cfg = FindConfigDev( cfg, ENC_MANUFACTURER, ENC_BOARD );
		if( !cfg )
			return 0;
		if( (APTR)cfg->cd_BoardAddr == db->db_Units[unit].duh_BASE )
			break;
	}

	if( (APTR)cfg->cd_BoardAddr != db->db_Units[unit].duh_BASE )
		return 0;

	if( db->db_Units[unit].duh_RefCount == 0 )
	{
		ObtainConfigBinding();

		ReleaseConfigBinding();
	}
	db->db_Units[unit].duh_RefCount++;
#endif
	return 1;
}


ASM SAVEDS LONG hw_ReleaseBoard( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                 ASMR(d0) ULONG unit                ASMREG(d0) )
{
#if 0
	if( unit > db->db_NBoards )
		return 0;

	for( i=0 ; i < db->db_NBoards ; i++ )
	{
		cfg = FindConfigDev( cfg, ENC_MANUFACTURER, ENC_BOARD );
		if( !cfg )
			return 0;
		if( (APTR)cfg->cd_BoardAddr == db->db_Units[unit].duh_BASE )
			break;
	}

#endif
	return 1;
}


/* store MAC address at "mac", 6 Bytes */
ASM SAVEDS LONG hw_GetMacAddress( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                  ASMR(d0) ULONG unit                ASMREG(d0),
				  ASMR(a1) UBYTE *mac                ASMREG(a1)
				)
{
	APTR boardbase = db->db_Units[unit].duh_BASE;

	enc624j6l_GetMAC( boardbase, mac );

	return 1;
}


/* prepare all boards such that a hw_attach() will succeed */
ASM SAVEDS LONG hw_Setup( ASMR(a0) DEVBASEP                  ASMREG(a0) )
{
	LONG ret = 0;
	struct HWData *hwd = &db->db_hwdat;

	InitSemaphore( &hwd->hwd_Sem );

	hwd->hwd_IntSig = AllocSignal(-1);
	hwd->hwd_Interrupt.is_Data = (0);

	if( InitIntervalTimer( &hwd->ivtimer ) > 0 )
	{
		hwd->hwd_SigMask = hwd->ivtimer.IVT_SigMask | (1<<(hwd->hwd_IntSig)) ;

		ret = 1;
	}

	return ret;
}

/* free resources, server is quitting */
ASM SAVEDS void hw_Shutdown(  ASMR(a0) DEVBASEP                  ASMREG(a0) )
{
	struct HWData *hwd = &db->db_hwdat;
	LONG i;

	/* safety measure */
	for( i=0 ; i < db->db_NBoards ; i++ )
		db->db_Units[i].duh_online = 0;
	myhw_ControlInterrupts( db );
	myhw_ControlIntervalTimer( db );


	if( hwd->hwd_IntSig >= 0 )
		FreeSignal( hwd->hwd_IntSig );

	ExitIntervalTimer( &hwd->ivtimer );
}


/* bring hardware online */
ASM SAVEDS LONG hw_Attach( ASMR(a0) DEVBASEP                  ASMREG(a0), 
                           ASMR(d0) ULONG unit                ASMREG(d0))
{
	struct HWData *hwd = &db->db_hwdat;
	APTR   boardbase = db->db_Units[unit].duh_BASE;
	LONG   ret = 1;
	ULONG  flags;


	flags = PIO_INIT_BROAD_CAST;
	if( hwd->multicast )
	     flags |= PIO_INIT_MULTI_CAST;

	if( hwd->fullduplex )
	    flags |= PIO_INIT_FULL_DUPLEX;

	if( hwd->flowcontrol )
	   flags |= PIO_INIT_FLOW_CONTROL;

	if( enc624j6l_Init( boardbase, flags ) <= 0 )
	   ret = 0; /* ERROR IN CASE OF DEVICE NOT FOUND */
	else
	{
		enc624j6l_SetMAC( boardbase, db->db_Units[unit].du_CFGAddr );
		enc624j6l_SetOnline( boardbase );

		db->db_Units[unit].duh_online = 1; /* used for int server and interval timer setup */

		myhw_ControlInterrupts( db ); /* start and/or stop (based on duh_online) */
		myhw_ControlIntervalTimer( db ); /* start and/or stop timer (based on duh_online) */
	}

	return ret;
}

/* put hardware into sleep mode (offline) */
ASM SAVEDS void hw_Detach( ASMR(a0) DEVBASEP                  ASMREG(a0), 
                           ASMR(d0) ULONG unit                ASMREG(d0))
{
	APTR   boardbase = db->db_Units[unit].duh_BASE;

	db->db_Units[unit].duh_online = 0; /* used for int server and interval timer setup */

	myhw_ControlInterrupts( db );    /* start and/or stop (based on duh_online) */
	myhw_ControlIntervalTimer( db ); /* start and/or stop timer (based on duh_online) */

	enc624j6l_SetOffline( boardbase );
	enc624j6l_Shutdown( boardbase );
}



ASM SAVEDS void hw_ConfigInit(   ASMR(a0) DEVBASEP                  ASMREG(a0) )
{
	struct HWData *hwd = &db->db_hwdat;

	hwd->timervalue  = HW_INTERVALDEF;
	hwd->fullduplex  = 0;
	hwd->flowcontrol = 0;
        hwd->multicast   = 0;
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
	if( args->flowcontrol )
		hwd->flowcontrol = 1;
	if( args->multicast )
		hwd->multicast = 1;
}

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
			     )
{
   /* TX with separate header support ? (optional) */
   enc624j6l_TransmitFrame( db->db_Units[unit].duh_BASE, frame, framesize
#ifdef HW_DMA_TX
                            ,header
#endif 
                          );
   return 1;
}


ASM SAVEDS LONG hw_recv_pending( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                 ASMR(d0) ULONG unit                ASMREG(d0) )
{
	APTR   board = db->db_Units[unit].duh_BASE;

	return enc624j6l_HaveRecv( board );
}


ASM SAVEDS LONG hw_recv_frame( ASMR(a0) DEVBASEP                  ASMREG(a0),
                               ASMR(d0) ULONG unit                ASMREG(d0),
			       ASMR(a1) UBYTE *frame              ASMREG(a1))
{
   LONG len;
   APTR board = db->db_Units[unit].duh_BASE;

   len = enc624j6l_RecvFrame( board, frame, 1522 );

   return len;
}


ASM SAVEDS ULONG hw_recv_sigmask( ASMR(a0) DEVBASEP                  ASMREG(a0) )
{
	struct HWData *hwd = &db->db_hwdat;

	return hwd->hwd_SigMask;
}

/* check if HW link parameters need update -> only when board is idle otherwise (no RX/TX) */
/* re-enables hardware interrupt */
ASM SAVEDS LONG hw_check_link_change( ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(d0) ULONG unit                ASMREG(d0) )
{
	APTR boardbase = db->db_Units[unit].duh_BASE;

	enc624j6l_CheckLinkChange( boardbase ); /* somewhere $e90000 and beyond */

	if( db->db_Units[unit].duh_online )
		enc624j6l_EnableGlobalInterrupt( boardbase );

	return 1; /* don't signal panic to the outside, everything is under control */
}

/* obtain running parameters of Ethernet MAC: link up/down, Speed, Duplex for 
          SANA-II DeviceQueryExtension (general parameters)
*/
ASM SAVEDS LONG hw_get_mac_status(    ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(d0) ULONG unit                ASMREG(d0) )
{
    struct HWData *hwd = &db->db_hwdat;
    APTR boardbase     = db->db_Units[unit].duh_BASE;

    LONG ret;
    LONG phstat3;
    LONG phstat1;
    LONG phcon1;

    enc624j6l_ReadPHY( boardbase, PHSTAT1 );
    phstat3 = enc624j6l_ReadPHY( boardbase, PHSTAT3 );
    phcon1  = enc624j6l_ReadPHY( boardbase, PHCON1 );
    phstat1 = enc624j6l_ReadPHY( boardbase, PHSTAT1 ); /* read again to see whether link is established */
    if( ( phstat1 < 0 ) || ( phstat3 < 0 ) )
        return HW_MAC_INVALID; /* no valid data found */

	/* a) check Link */
	ret = 0;
	if( phstat1 & PHSTAT1_LLSTAT )
	{
		ret |= HW_MAC_LINK_UP;
		
/*		if( phstat1 & PHSTAT1_ANDONE ) */ /* autonegotiation successful */
		if( phcon1  & PHCON1_ANEN ) /* autonegotiation enabled */
		 ret |= HW_MAC_AUTONEGOTIATION;

		phstat3 = (phstat3>>2)&7; /* get active Speed/Duplex */
		switch( phstat3 )
		{
			case 6:	
				if( hwd->flowcontrol )
					ret |= (HW_MAC_ON_FDX_FC<<HW_MACB_SPEED100);
				else
					ret |= (HW_MAC_ON_FDX<<HW_MACB_SPEED100);
				break;
			case 5:	
				if( hwd->flowcontrol )
					ret |= (HW_MAC_ON_FDX_FC<<HW_MACB_SPEED10);
				else
					ret |= (HW_MAC_ON_FDX<<HW_MACB_SPEED10);
				break;
			case 2:	ret |= (HW_MAC_ON<<HW_MACB_SPEED100);
				break;
			case 1:	ret |= (HW_MAC_ON<<HW_MACB_SPEED10);
				break;
			default: break;
		}		
	}

	return ret;

}

/* read PHY register */
ASM SAVEDS LONG hw_read_phy(          ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(d0) ULONG unit                ASMREG(d0),
				      ASMR(d1) ULONG reg                 ASMREG(d1) )
{
    APTR boardbase     = db->db_Units[unit].duh_BASE;

    return enc624j6l_ReadPHY( boardbase, reg );
}

/* write PHY register (return -1 in case of error) */
ASM SAVEDS LONG hw_write_phy(         ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(d0) ULONG unit                ASMREG(d0),
				      ASMR(d1) ULONG reg                 ASMREG(d1),
				      ASMR(d2) ULONG value               ASMREG(d2) )
{
    APTR boardbase     = db->db_Units[unit].duh_BASE;

    return enc624j6l_WritePHY( boardbase, reg, value );
}

/*
  calculate CRC32 for MAC address hashing
  slow but relatively short (and can be adapted to other 
  polynoms easily)
*/
ASM ULONG myhw_CRC32( ASMR(a0) UBYTE *buffer ASMREG(a0), 
                      ASMR(d0) ULONG nbytes  ASMREG(d0) )
{
	UWORD i,j;
	ULONG t,crc = 0xffffffff;
	ULONG poly  = 0x04c11db7; 

	for( j = 0 ; j < nbytes ; j++ )
	{
	  for( i = 0 ; i < 8 ; i++ )
	  {
		t     = crc>>31;
		crc <<= 1;
		if( ( t ^ ( buffer[j] >> i ) ) & 0x01 )
			crc ^= poly;
	  }
	}

	return crc;
}

#if 0
ASM ULONG CMPgeMAC( ASMR(a0) UBYTE *cur ASMREG(a0), 
                    ASMR(a1) UBYTE *cmp ASMREG(a1) )
{
 USHORT i;
 ULONG  res = 1;

 for( i = 0 ; i < 6 ; i++ )
 {
	if( cur[i] > cmp[i] )
		break;
	if( cur[i] < cmp[i] )
	{
		res = 0;
		break;
	}
 }
 return res;
}
#endif

ASM ULONG CMPleMAC( ASMR(a0) UBYTE *cur ASMREG(a0), 
                    ASMR(a1) UBYTE *cmp ASMREG(a1) )
{
 USHORT i;
 ULONG  res = 1;

 for( i = 0 ; i < 6 ; i++ )
 {
	if( cur[i] < cmp[i] )
		break;
	if( cur[i] > cmp[i] )
	{
		res = 0;
		break;
	}
 }
 return res;
}


ASM void INCMAC( ASMR(a0) UBYTE *curmc ASMREG(a0) )
{
 SHORT i;
	for( i=5 ; i >= 0 ; i-- )
	{
		curmc[i]++;
		if( curmc[i] != 0 )
		{
			break;
		}
	}
}

/* TODO: implement Multicast hashing */
ASM SAVEDS LONG hw_change_multicast(  ASMR(a0) DEVBASEP                  ASMREG(a0),
                                      ASMR(d0) ULONG unit                ASMREG(d0),
                                      ASMR(a1) struct List *mcastlist    ASMREG(a1) )
{
	APTR   boardbase = db->db_Units[unit].duh_BASE;
	struct HWData *hwd = &db->db_hwdat;
	ULONG  flags;
	ULONG  crc,curhash;
	USHORT mc_hashes[4] = {0,0,0,0}; /* EHT1, EHT2, EHT3, EHT4 */
	USHORT *hash_tab;
	struct MCastEntry *mc;
	UBYTE  *curmc;

	flags = PIO_INIT_BROAD_CAST;

	if( db->db_Units[0].du_Flags & DUF_PROMISC )
	    flags |= PIO_INIT_PROMISC;

	if( hwd->multicast ) /* accept ALL multicast ? (config option) */
	     flags |= PIO_INIT_MULTI_CAST;

	/* list non-empty ? */
	hash_tab = NULL;
	if( mcastlist->lh_Head->ln_Succ )
	{
	     /* flags |= PIO_INIT_MULTI_CAST; */ /* see above */
	     hash_tab = mc_hashes;

	      for( mc = (struct MCastEntry *)mcastlist->lh_Head; 
	           mc->mce_Node.mln_Succ ; 
	           mc=(struct MCastEntry*)mc->mce_Node.mln_Succ )
	      {
	     	curmc = mc->mce_start;
		while( CMPleMAC( curmc, mc->mce_stop ) )
		{
		     	 D(("CMPleMAC %ld %02lx%02lx%02lx%02lx%02lx%02lx - %02lx%02lx%02lx%02lx%02lx%02lx\n",\
		     	        CMPleMAC( curmc, mc->mce_stop ),\
				(ULONG)curmc[0],(ULONG)curmc[1],\
				(ULONG)curmc[2],(ULONG)curmc[3],\
				(ULONG)curmc[4],(ULONG)curmc[5],\
				(ULONG)mc->mce_stop[0],(ULONG)mc->mce_stop[1],\
				(ULONG)mc->mce_stop[2],(ULONG)mc->mce_stop[3],\
				(ULONG)mc->mce_stop[4],(ULONG)mc->mce_stop[5] \
			  ));

			crc     = myhw_CRC32( mc->mce_start, 6 );
			curhash = (crc & 0x1f800000)>>23; /* 2 bit reg, 4 bit hash pos in reg */
			mc_hashes[ ((curhash >> 4)&3) ] |= 1<<(curhash&0xf);
			INCMAC( curmc );
		}
	      }
	}
	
	D(("hw_change_multicast board %lx flags %lx hashtab %lx\n",(ULONG)boardbase,(ULONG)flags,(ULONG)hash_tab));

	enc624j6l_bc_mc_filter(boardbase, flags, hash_tab );/* flags & (PIO_INIT_BROAD_CAST|PIO_INIT_MULTI_CAST) ); */

	return	1;
}

/* global for all boards (relies on hwd_online) */
void myhw_ControlInterrupts( DEVBASEP ) 
{
	struct HWData *hwd = &db->db_hwdat;
	LONG i,flag;

	ObtainSemaphore( &hwd->hwd_Sem ); /* redundant here: server is a single task -> but it looks good :-) */

	flag = 0;
	for( i=0 ; i < db->db_NBoards ; i++ )
	{
		if( db->db_Units[i].duh_online )
		{
			hwd->hwd_act_boards[flag] = db->db_Units[i].duh_BASE;
			flag++;
		}
	}
	hwd->hwd_act_boards[flag]   = (0);                   /* NULL-terminate */
	hwd->hwd_act_boards[flag+1] = (APTR)FindTask(0);     /* append task    */
	hwd->hwd_act_boards[flag+2] = (APTR)hwd->hwd_IntSig; /* append sigbit  */
	hwd->hwd_act_boards[flag+3] = (APTR)db->db_SysBase;  /* append ExecBase *((APTR*)0x4); */

	if( flag )	/* determine on/off switch */
	{
		/* at least 1 board is online */
		if(  hwd->hwd_Interrupt.is_Data )
			flag = 0; /* no change */
		else	flag = 1; /* start timer */
	}
	else
	{
		if( hwd->hwd_Interrupt.is_Data )
			flag = -1; /* stop timer */
		else	flag =  0; /* no change */
	}

	/* flag is >0 = start, <0 = stop or 0 = no change */
	if( flag > 0 )
	{
	        hwd->hwd_Interrupt.is_Code = (void(*)())enc624j6l_IntServer_List;
	        hwd->hwd_Interrupt.is_Data = &hwd->hwd_act_boards[0];
	        hwd->hwd_Interrupt.is_Node.ln_Type = NT_INTERRUPT;
	        hwd->hwd_Interrupt.is_Node.ln_Pri = 126; /* 51; */
	        hwd->hwd_Interrupt.is_Node.ln_Name = (char*)intname;
	        AddIntServer( HW_INTSOURCE, &hwd->hwd_Interrupt );
	}

	for( i=0 ; i < db->db_NBoards ; i++ )
	{
			if( db->db_Units[i].duh_online )
				enc624j6l_EnableInterrupt( db->db_Units[i].duh_BASE, FindTask(0), (int)hwd->hwd_IntSig, 0 );
			else
		                enc624j6l_DisableInterrupt( db->db_Units[i].duh_BASE );
	}

	if( flag < 0 )
	{
                hwd->hwd_Interrupt.is_Data = (0);
                RemIntServer( HW_INTSOURCE, &hwd->hwd_Interrupt );
		hwd->hwd_Interrupt.is_Code = (0);
	}

	ReleaseSemaphore( &hwd->hwd_Sem );
}


/* global for all boards (relies on hwd_online) */
void myhw_ControlIntervalTimer( DEVBASEP ) 
{
	struct HWData *hwd = &db->db_hwdat;
	LONG i,flag;

	ObtainSemaphore( &hwd->hwd_Sem );

	flag = 0;
	for( i=0 ; i < db->db_NBoards ; i++ )
	{
		if( db->db_Units[i].duh_online )
			flag = 1;
	}
	if( flag )
	{
		/* at least 1 board is online */
		if(  hwd->ivtimer.IVT_Runflag )
			flag = 0; /* no change */
		else	flag = 1; /* start timer */
	}
	else
	{
		if(  hwd->ivtimer.IVT_Runflag )
			flag = -1; /* stop timer */
		else	flag =  0; /* no change */
	}
	if( hwd->timervalue < 1000 )
		flag = -1;

	if( flag > 0 )
		StartIntervalTimer( &hwd->ivtimer, hwd->timervalue );
	if( flag < 0 )
		StopIntervalTimer( &hwd->ivtimer );

	ReleaseSemaphore( &hwd->hwd_Sem );
}


