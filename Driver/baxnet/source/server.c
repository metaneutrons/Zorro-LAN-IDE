/*
  server.c

  (C) 2018 Henryk Richter <henryk.richter@gmx.net>

  Main Dispatcher

  TODO: sort multicast addresses and ranges

*/
#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <dos/dostags.h>
#include <devices/timer.h>

#include "device.h"
#include "hw.h"
#include "macros.h"
#include "server.h"


/* (avoid external link libraries) this call is not time critical */
static void MemSet( void *a, ULONG val, ULONG size )
{
	UBYTE *p = (UBYTE*)a;
	while( size-- )
		*p++ = (UBYTE)val;
}


static void server_AbortList( DEVBASEP, struct List*list )
{
	struct IOSana2Req *ioreq;

	while( (ioreq = (struct IOSana2Req *)GetHead( list )) )
	{
		Remove( (struct Node*)ioreq );
		ioreq->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
		ioreq->ios2_WireError = S2WERR_UNIT_OFFLINE;
		svTermIO( db, ioreq );
	}
}


/* cancel all pending requests */
static void server_Offline_CancelRequests( DEVBASEP , ULONG unit )
{
	struct SanaReadType *type;
	struct SanaReader   *rd;

	D(("server_Offline_CancelRequests for unit %ld\n",unit));

	ObtainSemaphore( &db->db_Units[unit].du_Sem );
	/* start with the read list -> types -> readers -> reqlist */
	type = (struct SanaReadType *)GetHead( &db->db_Units[unit].du_ReadTypes );
	while( type )
	{
		rd = (struct SanaReader *)GetHead( &type->srt_Readers );
		while( rd )
		{
			server_AbortList( db, &rd->snr_Requests );
			rd = GetSucc( rd );
		}
		type = GetSucc( type );
	}

	server_AbortList( db, (struct List*)&db->db_Units[unit].du_WriteQueue );
	server_AbortList( db, (struct List*)&db->db_Units[unit].du_EventQueue );
	server_AbortList( db, (struct List*)&db->db_Units[unit].du_ReadOrphans );

	ReleaseSemaphore( &db->db_Units[unit].du_Sem );

	/* clear Multicast list, TODO: subroutine */
	{
	 struct MCastEntry *entry;
	 
	 while(1)
	 {
	  entry = (struct MCastEntry *)db->db_svdat.sv_mclist.lh_Head;
	  if( !entry->mce_Node.mln_Succ )
	  	break;
	  REMOVE(entry);
	  FreeMem(entry,sizeof(struct MCastEntry));
	 }
	}


	D(("server_Offline_CancelRequests done\n"));
}


static LONG server_setOnline( DEVBASEP , ULONG unit, ULONG evt )
{
	D(("Unit %ld online command\n",unit));

	if( db->db_Units[unit].du_Flags & DUF_ONLINE )
	{
		D(("Was Online already\n"));
		return SERR_OK;
	}


	if( !hw_Attach( db, unit ) )
	{
		D(("Hardware hw_attach Fail\n"));
		return SERR_ERROR;
	}
	D(("hw_attach success\n"));

	{ /* track last opening time */
		struct timerequest timereq;
		struct Library *TimerBase; 

		MemSet( &timereq, 0, sizeof(struct timerequest));
		OpenDevice("timer.device", 0, (struct IORequest*)&timereq, 0);
		TimerBase = (struct Library *)timereq.tr_node.io_Device;
		GetSysTime( &db->db_Units[unit].du_DevStats.LastStart );
		CloseDevice( (struct IORequest*)&timereq);
	}
	db->db_Units[unit].du_Flags |= DUF_ONLINE;

	if( evt )
	{
		ObtainSemaphore( &db->db_Units[unit].du_Sem ); 
		server_SendEvent(db, unit, S2EVENT_ONLINE );
		ReleaseSemaphore( &db->db_Units[unit].du_Sem ); 
	}

	D(("Unit %ld online\n",unit));

	return SERR_OK;
}


static void server_setOffline( DEVBASEP , ULONG unit, ULONG evt )
{
	D(("Unit %ld offline command\n",unit));

	if( !(db->db_Units[unit].du_Flags & DUF_ONLINE ))
		return;

	hw_Detach( db,unit );

	db->db_Units[unit].du_Flags &= ~(DUF_ONLINE);

	if( evt )
	{
		ObtainSemaphore( &db->db_Units[unit].du_Sem ); 
		server_SendEvent(db, unit, S2EVENT_OFFLINE );
		ReleaseSemaphore( &db->db_Units[unit].du_Sem ); 
	}
	D(("Unit %ld offline\n",unit));
}


/* check for online status following closeDevice */
static void server_CloseDevice_offline( DEVBASEP )
{
	LONG unit;

 	unit=0;BOARDLOOP( ( ; unit < db->db_NBoards ; unit++ ) )
	{
		/* bring unit offline if no opener left */
		if( !db->db_Units[unit].du_OpenCount )
		{
			if( db->db_Units[unit].du_Flags & DUF_ONLINE )
			{
				server_setOffline( db, unit, 0 );
				server_Offline_CancelRequests( db, unit );
			}
		}
	}

}


/* IMPORTANT: base pointer is unavailable here until 
              retrieved from incoming message */
static struct ServerMsg *server_startup(void)
{
   struct ServerMsg *msg;
   struct Process *self;

   struct { struct Library *db_SysBase; } *db = (void*)0x4;

   self = (struct Process*)FindTask(NULL);

   D(("Waiting for Message from Device\n"));

   WaitPort( &self->pr_MsgPort);

   msg = (struct ServerMsg *)GetMsg( &self->pr_MsgPort );
 
   return msg;
}


/* real init, now with active device base */
static LONG server_init( DEVBASEP )
{
	LONG ret = SERR_ERROR;

	NEWLIST( &db->db_svdat.sv_mclist ); /* list of multicast addresses to accept on RX */

	db->db_ServerPort = CreateMsgPort(); /* forwarded IO Requests */
	if( db->db_ServerPort )
	{
		if( hw_Setup( db ) ) /* management stuff for hardware: interrupts, timers, memory etc. */
		{
			db->db_svdat.sv_framesize = ETH_MTU+18+8; /* 1514 = RAW IEE802.2, 1518 = with VLAN TAG, +8 for safety */
			db->db_svdat.sv_frame = AllocMem( db->db_svdat.sv_framesize, MEMF_PUBLIC );
			if( db->db_svdat.sv_frame )
				ret = SERR_OK;
		}
	}

	return ret;
}

static void server_up( DEVBASEP , struct ServerMsg *msg, LONG updown )
{
	msg->sm_result = updown;
	ReplyMsg( &msg->sm_msg );
	D(("Replied to Startup Message with %ld\n",updown));
}


static LONG server_shutdown( DEVBASEP )
{

	hw_Shutdown( db );

	if( (db->db_svdat.sv_framesize) && (db->db_svdat.sv_frame) )
		FreeMem( db->db_svdat.sv_frame, db->db_svdat.sv_framesize );
	db->db_svdat.sv_frame = (0);

	if( db->db_ServerPort )
		DeleteMsgPort(db->db_ServerPort);
	db->db_ServerPort = (0);

	return SERR_OK;
}


/* read config file (if present) and reconfigure HW parameters */
const BYTE configtemplate[] = COMMON_TEMPLATE HW_CONFIGTEMPLATE;
static LONG server_Config( DEVBASEP )
{
	BPTR cfgfile;
	LONG i;

	hw_ConfigInit( db ); /* hw defaults */

	if((cfgfile = Open(HW_CONFIGFILE, MODE_OLDFILE))) /* see hwprivate.h for CONFIGFILE */
	{
		struct HWConfig args; /* see hwprivate.h */
		struct RDArgs *readargs;

		cfgfile = SelectInput(cfgfile); /* get default Input */
	
		MemSet( &args, 0, sizeof( args ) );

		readargs = ReadArgs( (BYTE*)configtemplate, (LONG *)&args, NULL);

		Close( SelectInput( cfgfile ) ); /* restore default input and retrieve cfgfile */

		if( readargs )
		{
			/* copy common variables */
			if(args.common.priority)
			 	SetTaskPri((struct Task*)db->db_ServerProc,BOUNDS(*args.common.priority, MINPRIORITY, MAXPRIORITY));

			i=0;BOARDLOOP( ( ; i < db->db_NBoards ; i++ ) )
			{
				if( args.common.bps )
					db->db_Units[i].du_BitPerSec = *args.common.bps;
				if( args.common.mtu )
					db->db_Units[i].du_MTU = *args.common.mtu;
			}

#if 0			/* ignored for now but kept in template for compatibility with old codebase */
			if(args.common.nospecialstats)
#endif
			hw_ConfigUpdate( db, &args );

			FreeArgs( readargs );
		}
	}
	
	return SERR_OK;
}

static LONG CMPeqMAC( BYTE *a, BYTE *b )
{
 LONG i;
 for( i=0 ; i<6 ; i++ )
 {
 	if( a[i] != b[i] )
 		return 0;
 }
 return 1;
}

static LONG server_changeMulticast( DEVBASEP, ULONG unit, struct IOSana2Req *ioreq, ULONG add_del )
{
	/* step 1: find start address in active list */
	struct MCastEntry *entry;
	LONG ret = 1;
	
	entry = (struct MCastEntry *)db->db_svdat.sv_mclist.lh_Head;
	while( entry->mce_Node.mln_Succ )
	{
		if( CMPeqMAC( entry->mce_start, ioreq->ios2_SrcAddr ) )
			break;
		entry = (struct MCastEntry *)entry->mce_Node.mln_Succ;
	}
		
	/* did we have a prior entry ? */
	if( entry->mce_Node.mln_Succ )
	{
		if( add_del )
		{
			entry->mce_refcount++;
			goto apply;
		}
		else
		{
			if( entry->mce_refcount > 1 )
			{
				entry->mce_refcount--;
				goto apply;
			}
			else
			{
				REMOVE(entry);
				FreeMem( entry, sizeof( struct MCastEntry ) );
				goto apply;
			}
		}
	}
	
	/* prior not found */
	if( !add_del )
	{
		ret = 0;	/* ERROR: CAN`T DELETE NONEXISTENT ENTRY */
		goto donothing;
	}

	/* add new entry */
	entry = (struct MCastEntry*)AllocMem( sizeof( struct MCastEntry ), MEMF_PUBLIC|MEMF_CLEAR );
	if( !entry )
	{
		ret = 0;
		goto donothing;
	}

	entry->mce_refcount = 1;
	COPYMAC( entry->mce_start, ioreq->ios2_SrcAddr );
	COPYMAC( entry->mce_stop,  ioreq->ios2_SrcAddr );

	ADDTAIL( &db->db_svdat.sv_mclist, entry );
	
apply:
	hw_change_multicast( db, unit, &db->db_svdat.sv_mclist );
	
donothing:
	return ret;
}



/* forwarded commands (online/offline etc.) */
static LONG server_HandleS2Commands( DEVBASEP )
{
	ULONG unit;
	struct IOSana2Req *ioreq;

	while( (ioreq = (struct IOSana2Req *)GetMsg(db->db_ServerPort)) )
	{
		unit = (ULONG)ioreq->ios2_Req.io_Unit;
		
		ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
		ioreq->ios2_WireError    = S2WERR_GENERIC_ERROR;

		switch( ioreq->ios2_Req.io_Command)
		{
			case S2_ONLINE:
				if( server_setOnline(db,unit,1 ) != SERR_OK )
				{
					ioreq->ios2_Req.io_Error = S2ERR_NO_RESOURCES;
					ioreq->ios2_WireError    = S2WERR_GENERIC_ERROR;
				}
				break;
			case S2_OFFLINE:
				server_setOffline( db, unit, 1 ); /* ,1: send event to opener */
				server_Offline_CancelRequests( db, unit );
				break;
			case S2_CONFIGINTERFACE:
				COPYMAC( db->db_Units[unit].du_CFGAddr, ioreq->ios2_SrcAddr );

				if( db->db_Units[unit].du_Flags & DUF_ONLINE )
					server_setOffline( db, unit,0 );

				server_setOnline( db, unit,0 );
				break;
				
			case S2_ADDMULTICASTADDRESS:
				if( server_changeMulticast( db, unit, ioreq, 1 ) )
				{
					ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
					ioreq->ios2_WireError    = S2WERR_GENERIC_ERROR;
				}
				else
				{
					ioreq->ios2_Req.io_Error = S2ERR_NO_RESOURCES;
					ioreq->ios2_WireError    = S2WERR_GENERIC_ERROR;
				}
				break;
				
			case S2_DELMULTICASTADDRESS:
				if( server_changeMulticast( db, unit, ioreq, 0 ) )
				{
					ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
					ioreq->ios2_WireError    = S2WERR_GENERIC_ERROR;
				}
				else
				{
					ioreq->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
					ioreq->ios2_WireError    = S2WERR_GENERIC_ERROR;
				}
				break;

			default: /* shouldn't happen */
				ioreq->ios2_Req.io_Error = S2ERR_NOT_SUPPORTED;
				ioreq->ios2_WireError = S2WERR_GENERIC_ERROR;
				break;
		}
		svTermIO( db, ioreq );
	}


	return SERR_OK;
}



/* error conditions sent via EventQueue */
static LONG server_SendEvent( DEVBASEP, ULONG unit, ULONG evtmask )
{
	struct IOSana2Req *ioreq;

	/* Semaphore is locked outside */
	/* ObtainSemaphore( &db->db_Units[unit].du_Sem ); */

	for( ioreq = (struct IOSana2Req *)db->db_Units[unit].du_EventQueue.lh_Head;
             (ioreq->ios2_Req.io_Message.mn_Node.ln_Succ);
	     ioreq = (struct IOSana2Req *)ioreq->ios2_Req.io_Message.mn_Node.ln_Succ )
	{
		if( ioreq->ios2_WireError & evtmask )
		{
			Remove( (struct Node*)ioreq );
			svTermIO( db, ioreq );
			break;
		}

	}
	/* ReleaseSemaphore( &db->db_Units[unit].du_Sem ); */
	return SERR_OK;
}



/* handle write errors */
LONG server_writeerror( DEVBASEP, ULONG unit, struct IOSana2Req *ioreq, LONG code )
{
	if (code == SERR_BUFFER_ERROR)  /* no buffer management callback or copy error */
	{
		D(("buffer error (TX)\n"));
		server_SendEvent(db, unit, S2EVENT_ERROR | S2EVENT_BUFF | S2EVENT_SOFTWARE);
		/* db->db_SpecialStats[S2SS_TXERRORS].Count++; */

		ioreq->ios2_Req.io_Error = S2ERR_SOFTWARE;
		ioreq->ios2_WireError = S2WERR_BUFF_ERROR;
	}
	else /* if (code != SERR_OK) */
	{
		D(("TX error\n"));
		server_SendEvent(db, unit, S2EVENT_ERROR | S2EVENT_TX | S2EVENT_HARDWARE);
		/* db->db_SpecialStats[S2SS_TXERRORS].Count++; */

		ioreq->ios2_Req.io_Error = S2ERR_TX_FAILURE;
		ioreq->ios2_WireError = S2WERR_GENERIC_ERROR;
	}

	Remove((struct Node*)ioreq);
	svTermIO( db, ioreq ); /* SANA2IOF_QUICK is off for write queue, just ReplyMsg */ 

	return 0;
}


#ifndef EXTERNAL_WRITE_FRAME
/* note: entering here with locked semaphore */
static LONG write_frame( DEVBASEP, ULONG unit, struct IOSana2Req *ioreq )
{
	LONG ret = SERR_OK;
	UBYTE *copy_ptr,*frame;
	ULONG framesize;
	struct db_BufferManagement *dbm;

	   D(("write: type %08lx, size %ld\n",ioreq->ios2_PacketType,
        	                              ioreq->ios2_DataLength));

	frame     = db->db_svdat.sv_frame;
	framesize = ioreq->ios2_DataLength;
	copy_ptr = frame;

	/* copy raw frame: simply overwrite ethernet frame part of plip packet */
	if( !(ioreq->ios2_Req.io_Flags & SANA2IOF_RAW) ) 
	{
		COPYMAC( frame,   ioreq->ios2_DstAddr ); /* macro in server.h */
		COPYMAC( frame+6, db->db_Units[unit].du_CFGAddr );
		*( (USHORT*)(frame+12)) = ioreq->ios2_PacketType;
		copy_ptr += 14;
		framesize += 14;
	} 
	
   	dbm = (struct db_BufferManagement *)ioreq->ios2_BufferManagement;

	/* copy32 is valid ptr, even if the stack offers only regular CopyFromBuffer */
	if( (*dbm->dbm_CopyFromBuffer32)(copy_ptr,ioreq->ios2_Data, ioreq->ios2_DataLength) )
 	{
send:
		D(("+hw_send\n"));
		ret = hw_send_frame(db, unit, frame, framesize) ? SERR_OK : SERR_ERROR;
		D(("-hw_send\n"));
	}
	else
	{
		/* if copy32 fails, try regular call */
		if( (*dbm->dbm_CopyFromBuffer)(copy_ptr,ioreq->ios2_Data, ioreq->ios2_DataLength) )
			goto	send;
		else
			ret = SERR_BUFFER_ERROR;
	}

	return ret;
}
#endif


static LONG server_writequeue( DEVBASEP, ULONG unit )
{
	LONG code;
	struct IOSana2Req *ioreq,*nextio;

	ObtainSemaphore( &db->db_Units[unit].du_Sem );

#if 1
	/* whole queue per call */
	for(  ioreq = (struct IOSana2Req *)db->db_Units[unit].du_WriteQueue.lh_Head;
	     (nextio= (struct IOSana2Req *)ioreq->ios2_Req.io_Message.mn_Node.ln_Succ);
	      ioreq = nextio )
#else
	/* single write per call */
	ioreq = (struct IOSana2Req *)db->db_Units[unit].du_WriteQueue.lh_Head;
	if( ioreq->ios2_Req.io_Message.mn_Node.ln_Succ )
#endif
	{
		code = write_frame( db, unit, ioreq );
		if( code >= 0 )
		{
			/* statistics ? */
			if( db->db_Units[unit].du_Flags & DUF_TYPETRACK )
			{
				struct SanaReadType *type;
			 	type = (struct SanaReadType *)GetHead( &db->db_Units[unit].du_ReadTypes );
			 	while( type )
			 	{
					if( type->srt_Type == ioreq->ios2_PacketType )
					{
						type->srt_Sana2PacketTypeStats.PacketsSent++;
						type->srt_Sana2PacketTypeStats.BytesSent += ioreq->ios2_DataLength;
						break;
					}
					type = GetSucc( type );
			 	}
			}

			db->db_Units[unit].du_DevStats.PacketsSent++;
		        ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
	        	ioreq->ios2_WireError = S2WERR_GENERIC_ERROR;
		        REMOVE( (struct Node*)ioreq );
			svTermIO( db, ioreq ); /* SANA2IOF_QUICK is off for write queue, just ReplyMsg */ 
		}
		else
		{
			/* error handling */
			server_writeerror( db, unit, ioreq, code );
		}
	}

	ReleaseSemaphore( &db->db_Units[unit].du_Sem );

	return SERR_OK;
}


/* note: entering here with locked semaphore */
static void server_read_frame( DEVBASEP, ULONG unit, struct IOSana2Req *ioreq, UBYTE *frame, ULONG framesize )
{
	struct db_BufferManagement *dbm;
	ULONG  bcflag = 0;

	COPYMAC( ioreq->ios2_DstAddr, frame ); /* macro in server.h */
	COPYMAC( ioreq->ios2_SrcAddr, frame+6 );

	if( *frame & 1 ) /* just check Ethernet MULTICAST flag */
	{
		ULONG mac = *((ULONG*)frame );

		bcflag = SANA2IOF_MCAST;
		if( mac == 0xffffffff ) /* evil: ignore the lower 16 bit... */
		 	bcflag = SANA2IOF_BCAST; /* special case of multicast, actually */
#if 0
		mac >>= 8;
		if( mac == 0x0001005e ) /* IPv4 MCAST */
			bcflag = SANA2IOF_MCAST;
#endif
	}

	if( !(ioreq->ios2_Req.io_Flags & SANA2IOF_RAW) )
	{
		frame     += 14; /* skip DST,SRC,Type */
		framesize -= 18; /* also remove CRC */
		D(("plain mode: frame flags %ld\n",bcflag));
	}
	ioreq->ios2_Req.io_Flags &= SANA2IOF_RAW; /* ignore all other flags */
	ioreq->ios2_Req.io_Flags |= bcflag;
	ioreq->ios2_Req.io_Error  = 0;
	ioreq->ios2_WireError     = 0;

	dbm = (struct db_BufferManagement *)ioreq->ios2_BufferManagement;
	D(("ReadFrm IO %lx, DBM %lx frm %lx frmsz %ld\n",(ULONG)ioreq,(ULONG)dbm,(ULONG)frame,framesize));
	if(!(*dbm->dbm_CopyToBuffer32)(ioreq->ios2_Data, frame, framesize ))
 	{
		if(!(*dbm->dbm_CopyToBuffer)(ioreq->ios2_Data, frame, framesize ))
		{
			ioreq->ios2_Req.io_Error = S2ERR_SOFTWARE;
			ioreq->ios2_WireError = S2WERR_BUFF_ERROR;
		}
	}
	D(("Copied io_Error %ld\n",(LONG)ioreq->ios2_Req.io_Error));

	Remove( (struct Node*)ioreq );
	svTermIO( db, ioreq );
}


/* it is assumed that hw_recv_pending() was called before entering here */
static LONG server_readqueue( DEVBASEP, ULONG unit )
{
	struct SanaReadType *type;
	struct SanaReader *rd;
	struct IOSana2Req *ioreq;
	UBYTE *frame = db->db_svdat.sv_frame;
	LONG   framesize;
	LONG   frametype;
	LONG   dropflag;

	while(1) 
	{
		if( (framesize = hw_recv_frame(db, unit, frame) ) > 0 )
		{
			/* TODO: devstats */
			frametype = *( (USHORT*)(frame+12) );

			D(("Have Frame size %ld type %ld\n",framesize,frametype));

			ObtainSemaphore( &db->db_Units[unit].du_Sem );

			 type = (struct SanaReadType *)db->db_Units[unit].du_ReadTypes.lh_Head;
			 while( (type->srt_Node.mln_Succ) )
			 {
				if( type->srt_Type == frametype )
					goto havetype;
				type = (struct SanaReadType *)type->srt_Node.mln_Succ;
			 }

			 	 /* desired type not found, get orphan list */
				 db->db_Units[unit].du_DevStats.UnknownTypesReceived++;
orphan:
				 ioreq = (struct IOSana2Req *)GetHead( (struct List*)&db->db_Units[unit].du_ReadOrphans );
				 if( ioreq )
				 {
					D(("Orphaned Frame\n"));

				 	ioreq->ios2_PacketType = frametype;
				 	server_read_frame( db, unit, ioreq, frame, framesize );
				 }
				 else
				 {
					D(("No ioreq for orphaned frame\n"));
				 }

				 goto nextframe;

havetype:
			 /* push frame to all readers */
			 /*if( type->srt_Flags & SRTF_TRACKTYPE )*/ /* stats ? */
			 {
				/* type->srt_Sana2PacketTypeStats.PacketsSent++; */
				/* type->srt_Sana2PacketTypeStats.BytesSent++; */
				/* type->srt_Sana2PacketTypeStats.PacketsDropped++; */

				type->srt_Sana2PacketTypeStats.PacketsReceived++;
				type->srt_Sana2PacketTypeStats.BytesReceived += framesize;
			 }
			 db->db_Units[unit].du_DevStats.PacketsReceived++;

			 rd = (struct SanaReader *)type->srt_Readers.lh_Head;
			 dropflag = 1;
			 while( rd->snr_Node.mln_Succ )
			 {
				ioreq = (struct IOSana2Req *)rd->snr_Requests.lh_Head; 
				if( ioreq->ios2_Req.io_Message.mn_Node.ln_Succ )
				{
					server_read_frame( db, unit, ioreq, frame, framesize );
					dropflag = 0;
				}
				rd = (struct SanaReader *)rd->snr_Node.mln_Succ;
			 }
			 if( dropflag )
			 {
			 	db->db_Units[unit].du_DevStats.Overruns++;
				type->srt_Sana2PacketTypeStats.PacketsDropped++;
				D(("No Reader for frame\n"));
				goto orphan; /* does it help if we give the frame to the orphan list? */
			 }
nextframe:
			 ReleaseSemaphore( &db->db_Units[unit].du_Sem );
		}
		else
		{
			/* TODO: check for error, send S2EVENT_HARDWARE|S2EVENT_ERROR|S2EVENT_RX along with stats db_DevStats.BadData */
			goto end;
		}
	}
end:
	return SERR_OK;
}



VOID SAVEDS ServerTask(void)
{
 struct ServerMsg *msg;
 ULONG wmask,gotmask,spcmask,portsigmask,recvmask,i=0,action;
 struct IOSana2Req*wr;
 struct DevUnit *curunit;
 DEVBASEP = (0); /* Don`t call Libraries until server_startup() succeeds! */

 msg = server_startup();
 if( !msg ) 
	goto QuitServer; /* unlikely */
 db  = msg->sm_devbase;

 if( !server_init( db ) )
 {
	server_up( db, msg, 0 );
 	goto QuitServer;
 }

 server_Config( db );     /* load config (if present)    */
 server_up( db, msg, 1 ); /* init done, server is active */

 /*----------------------------------------------------------------------*/
 /*--------------------------  MAIN LOOP --------------------------------*/
 /*----------------------------------------------------------------------*/
 portsigmask = 1<<db->db_ServerPort->mp_SigBit;
 recvmask    = hw_recv_sigmask( db );
 wmask  = SIGBREAKF_CTRL_F|SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D|portsigmask|recvmask;
 spcmask= SIGBREAKF_CTRL_C|portsigmask|SIGBREAKF_CTRL_D;

 D(("Portsigmask %ld RecvMask %ld wmask %ld\n",portsigmask,recvmask,wmask));

 action  = 0;
 gotmask = 0;
 while(1)
 {
 	if( !action ) /* queues quiet ? -> sleep */
	{
	 	i=0;BOARDLOOP( ( ; i < db->db_NBoards ; i++ ) )
		{ 	/* housekeeping -> check for link change on idle, re-enable interrupts etc. */
			hw_check_link_change(db,i);
		}
	 	gotmask = Wait(wmask);
	}

	action  = 0; /* any action on the queues ? */
	curunit = &db->db_Units[0];
	i=0;BOARDLOOP( (; i < db->db_NBoards ; i++ ) ) /* skip the for() when just a single device is supported */
	{
	 if( curunit->du_Flags & DUF_ONLINE )
	 {
		/* D(("BoardLoop %ld\n",i)); */
		wr = (struct IOSana2Req *)curunit->du_WriteQueue.lh_Head; /* don't bother with semaphore, lh_Head is always !=0 */
		if( wr->ios2_Req.io_Message.mn_Node.ln_Succ ) /* quick check for write reqs */
		{
			server_writequeue( db, i );
			action = 1;
		}
		if( hw_recv_pending(db,i) )
		{
			D(("Read on unit %ld start\n",i));
			server_readqueue( db, i );
			D(("Read on unit %ld stop\n",i));
			action = 1;
		}
	 }
	 curunit++;
	}

	if( gotmask & spcmask ) /* rare flags */
	{
		if( gotmask & portsigmask )
		{
			D(("SANA-II request handling commencing\n"));
			server_HandleS2Commands( db );
			gotmask &= ~portsigmask;
		}
		if( gotmask & SIGBREAKF_CTRL_D )
		{
			D(("Got CTRL-D in Server. Checking Online status\n"));
			gotmask &= ~SIGBREAKF_CTRL_D;
			server_CloseDevice_offline( db );
		}
		if( gotmask & SIGBREAKF_CTRL_C )
		{
			D(("Got CTRL-C in Server. leaving.\n"));
			break;
		}
	}
 }

 server_shutdown( db );

QuitServer:
 if( db )
 	db->db_ServerProc = (0);

 { /* revert to "real" execbase, in case that we never got the startup msg */
  struct { struct Library *db_SysBase; } *db = (void*)0x4;
  D(("Server exiting"));
  Forbid();
 }
}

