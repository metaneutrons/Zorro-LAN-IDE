/*
  device.c

  (C) 2018 Henryk Richter <henryk.richter@gmx.net>

  Device Functions

  the device core consists of three files, to be linked
  in the following order
  - deviceheader.c - ROMTAG
  - deviceinit.c   - strings and init tables
  - device.c       - functions
  I've had seen quite creative ways the different compilers
  handled the order of objects in the linked result and that's
  why I've chosen to break it down in the way you see here.

  The request handling and hardware interaction is carried out
  in server.c as a separate process. Interrupts are forwarded
  as signals to server.c as well (depending on target hardware).

*/
#define DEVICE_MAIN

#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <dos/dostags.h>
#include <utility/tagitem.h>
#include <exec/lists.h>
#ifdef HAVE_VERSION_H
#include "version.h"
#endif
/* NSD support is optional */
#ifdef NEWSTYLE
#include <devices/newstyle.h>
#include <devices/sana2devqueryext.h> /* new extension, code can be built without this include (but also without ext query then...) */

const UWORD dev_supportedcmds[] = {
	NSCMD_DEVICEQUERY,
	CMD_READ,
	CMD_WRITE,
	S2_READORPHAN,
	S2_BROADCAST,
	S2_MULTICAST,
	S2_ONEVENT,
	S2_ONLINE,
	S2_OFFLINE,
	S2_CONFIGINTERFACE,
	S2_GETSTATIONADDRESS,
	S2_DEVICEQUERY,
	S2_TRACKTYPE,
	S2_UNTRACKTYPE,
	S2_GETTYPESTATS,
	S2_GETGLOBALSTATS,
	S2_ADDMULTICASTADDRESS,
	S2_DELMULTICASTADDRESS,
#ifdef S2_DEVICEQUERYEXT
	S2_DEVICEQUERYEXT,
#endif
	0
};

const struct NSDeviceQueryResult NSDQueryAnswer = {
	0,
	16, /* up to SupportedCommands (inclusive) */
	NSDEVTYPE_SANA2,
	0,  /* subtype */
	(UWORD*)dev_supportedcmds
};
#endif

#include "device.h"
#include "hw.h"
#include "macros.h"





ASM SAVEDS struct Device *DevInit( ASMR(d0) DEVBASEP                  ASMREG(d0), 
                                   ASMR(a0) BPTR seglist              ASMREG(a0), 
				   ASMR(a6) struct Library *_SysBase  ASMREG(a6) )
{
	UBYTE*p;
	ULONG i;
	LONG  ok;

	p = ((UBYTE*)db) + sizeof(struct Library);
	i = sizeof(DEVBASETYPE)-sizeof(struct Library);
	while( i-- )
		*p++ = 0;

	db->db_SysBase = _SysBase;
	db->db_SegList = seglist;
	db->db_Flags   = 0;

	dbNewList( &db->db_BufferManagement );

	for(i=0 ; i < MAX_UNITS ; i++ )
	{
		dbNewList( (struct List*)&db->db_Units[i].du_ReadTypes   );
		dbNewList( (struct List*)&db->db_Units[i].du_ReadOrphans );
		dbNewList( (struct List*)&db->db_Units[i].du_WriteQueue  );
		dbNewList( (struct List*)&db->db_Units[i].du_EventQueue  );
		InitSemaphore( &db->db_Units[i].du_Sem );
		db->db_Units[i].du_MTU = DEF_MTU;
		db->db_Units[i].du_BitPerSec = DEF_BPS;
	}

	ok = 0;
	if( (DOSBase = OpenLibrary("dos.library", 36)) )
	{
		if( (UtilityBase = OpenLibrary("utility.library", 37)) )
		{
			/* Search HW */
			ok = hw_Find_Boards( db );
			if( ok > MAX_UNITS )
				ok = MAX_UNITS;
			if( !ok )
			{
				D(("No Hardware\n"));
				CloseLibrary(DOSBase);
				CloseLibrary(UtilityBase);
			}
			db->db_NBoards = ok;
		}
		else
		{
			D(("No Utility\n"));
			CloseLibrary(DOSBase);
		}
	}
	else
	{
		D(("No DOS\n"));
	}

	/* no hardware found, reject init */
	return (ok > 0) ? (struct Device*)db : (0);
}




ASM SAVEDS LONG DevOpen( ASMR(a1) struct IOSana2Req *ioreq           ASMREG(a1), 
                         ASMR(d0) ULONG unit                         ASMREG(d0), 
                         ASMR(d1) ULONG flags                        ASMREG(d1),
                         ASMR(a6) DEVBASEP                           ASMREG(a6) )
{
	LONG ok,ret = IOERR_OPENFAIL;

	D(("DevOpen for %ld\n",unit));

	db->db_Lib.lib_OpenCnt++; /* avoid Expunge, see below for separate "unit" open count */
	db->db_ID++; /* opener ID (always >0) */

	if( unit < db->db_NBoards )
	{
		ObtainSemaphore( &db->db_Units[unit].du_Sem ); /* lock current board */

		ok = 1;
		if( (db->db_Units[unit].du_OpenCount > 0) &&
		    (flags & SANA2OPF_MINE) )
			ok = 0;

		if( (db->db_Units[unit].du_OpenCount > 0) &&
		    (flags & SANA2OPF_PROM) )
			ok = 0;


		if( db->db_Units[unit].du_Flags & DUF_EXCLUSIVE )
		 	ok = 0;

		if( ok )
			ok = hw_AllocBoard( db, unit );

		if( ok )
		{
			struct db_BufferManagement *dbm;

			db->db_Units[unit].du_OpenCount++;
			db->db_Units[unit].du_Flags &= ~DUF_EXCLUSIVE;
			if( flags & SANA2OPF_MINE )
				db->db_Units[unit].du_Flags |= DUF_EXCLUSIVE;
		        if( flags & SANA2OPF_PROM ) /* implies _MINE by spec */
				db->db_Units[unit].du_Flags |= DUF_PROMISC;

			hw_GetMacAddress( db, unit, &db->db_Units[unit].du_HWAddr[0] );
			hw_GetMacAddress( db, unit, &db->db_Units[unit].du_CFGAddr[0] );

			ok = 0;
			dbm = (struct db_BufferManagement *)AllocMem( sizeof(struct db_BufferManagement),MEMF_ANY|MEMF_CLEAR);
			if( dbm )
			{
				dbm->dbm_CopyToBuffer = (BMCALL)((void (*)())(GetTagData(S2_CopyToBuff, 0,
			                                        (struct TagItem *)ioreq->ios2_BufferManagement)));
				dbm->dbm_CopyFromBuffer = (BMCALL)((void (*)())(GetTagData(S2_CopyFromBuff, 0,
				                                (struct TagItem *)ioreq->ios2_BufferManagement)));

				/* v1.95: disable CopyToBuffer32 due to problems with MiamiDX */
				/* MiamiDX requires byte count to be a multiple of 4 */
#if 1
				dbm->dbm_CopyToBuffer32 = (0);
				dbm->dbm_CopyFromBuffer32 = (0);
#else
				dbm->dbm_CopyToBuffer32 = (BMCALL)((void (*)())(GetTagData(S2_CopyToBuff32, 0,
			                                        (struct TagItem *)ioreq->ios2_BufferManagement)));
				dbm->dbm_CopyFromBuffer32 = (BMCALL)((void (*)())(GetTagData(S2_CopyFromBuff32, 0,
				                                (struct TagItem *)ioreq->ios2_BufferManagement)));
#endif
				if( !dbm->dbm_CopyToBuffer32 )
					dbm->dbm_CopyToBuffer32 = dbm->dbm_CopyToBuffer;
				if( !dbm->dbm_CopyFromBuffer32 )
					dbm->dbm_CopyFromBuffer32 = dbm->dbm_CopyFromBuffer;
					
				dbm->dbm_ID = db->db_ID;
				ok = 1;
			}
			if( ok )
			{
				/* start server if necessary */
				ok = dbStartServer( db, unit );
			}
			if( ok )
			{
				ret = 0;
				AddTail((struct List *)&db->db_BufferManagement,(struct Node *)dbm);
			        db->db_Lib.lib_Flags &= ~LIBF_DELEXP;
				ioreq->ios2_BufferManagement = (VOID *)dbm;
				ioreq->ios2_Req.io_Error  = 0;
				ioreq->ios2_Req.io_Unit   = (struct Unit *)unit;
				ioreq->ios2_Req.io_Device = (struct Device *)db;

				dbAddType( db, unit, 0x800 ); /* register IP by default */

			}
			else
			{
				db->db_Units[unit].du_OpenCount--;
				db->db_Units[unit].du_Flags &= ~DUF_EXCLUSIVE;
				hw_ReleaseBoard( db, unit );
				if( dbm ) FreeMem( dbm, sizeof(struct db_BufferManagement) );
			}
		}

		ReleaseSemaphore( &db->db_Units[unit].du_Sem );
	}

	if( ret == IOERR_OPENFAIL )
	{
		ioreq->ios2_Req.io_Unit   = (0);
		ioreq->ios2_Req.io_Device = (0);
		ioreq->ios2_Req.io_Error  = ret;
		db->db_Lib.lib_OpenCnt--;
	}
	ioreq->ios2_Req.io_Message.mn_Node.ln_Type = NT_REPLYMSG;

	D(("DevOpen return code %ld\n",ret));

	return ret;
}


ASM SAVEDS BPTR DevClose(   ASMR(a1) struct IOSana2Req *ioreq        ASMREG(a1),
                            ASMR(a6) DEVBASEP                        ASMREG(a6) )
{
	ULONG unit;
	BPTR  ret = (0);

	D(("DevClose open count %ld\n",db->db_Lib.lib_OpenCnt));

	/* request valid ? */
	if( !ioreq )
		return ret;
	if( (unit = (ULONG)ioreq->ios2_Req.io_Unit) > db->db_NBoards)
		return ret;


	ObtainSemaphore( &db->db_Units[unit].du_Sem ); /* lock current board */

	dbDeleteBuffers( db, ioreq ); /* buffer management and internal buffers */

	db->db_Lib.lib_OpenCnt--;

	if( db->db_Units[unit].du_OpenCount ) /* sanity */
		db->db_Units[unit].du_OpenCount--;

	ReleaseSemaphore( &db->db_Units[unit].du_Sem );

	/* clear flags */
	if( !(db->db_Units[unit].du_OpenCount) )
	{
		db->db_Units[unit].du_Flags &= ~(DUF_EXCLUSIVE|DUF_PROMISC);
	}

	/* give server time to offline down the hardware */
	if( (db->db_ServerProc) &&
	   !(db->db_Units[unit].du_OpenCount) &&
	    (db->db_Units[unit].du_Flags & DUF_ONLINE )
	  )
	{
		Signal( (struct Task*)db->db_ServerProc, SIGBREAKF_CTRL_D);
		Delay(1);
	}
	
	ObtainSemaphore( &db->db_Units[unit].du_Sem ); /* lock current board */

	hw_ReleaseBoard( db, unit );

	ioreq->ios2_Req.io_Device = (0);
	ioreq->ios2_Req.io_Unit   = (struct Unit *)(-1);

	ReleaseSemaphore( &db->db_Units[unit].du_Sem );

	if (db->db_Lib.lib_Flags & LIBF_DELEXP)
		ret = DevExpunge(db);

	return ret;
}


ASM SAVEDS BPTR DevExpunge( ASMR(a6) DEVBASEP                        ASMREG(a6) )
{
	BPTR seglist = db->db_SegList;
	ULONG i;
	struct SanaReadType *type;

	if( db->db_Lib.lib_OpenCnt )
	{
		db->db_Lib.lib_Flags |= LIBF_DELEXP;
		return (0);
	}
	
	Forbid();
	Remove((struct Node*)db); /* remove device node from system */
	Permit();

	dbStopServer(db,0);

	for( i=0 ; i < db->db_NBoards ; i++ )
	{
		while( (type = GetHead( &db->db_Units[i].du_ReadTypes ) ))
		{
			dbDeleteType( db, i, type->srt_Type );
		}

	}

	/* last steps: libraries */
	CloseLibrary(DOSBase);
	CloseLibrary(UtilityBase);
	FreeMem( ((BYTE*)db)-db->db_Lib.lib_NegSize,(ULONG)(db->db_Lib.lib_PosSize + db->db_Lib.lib_NegSize));

	return seglist;
}


ASM SAVEDS VOID DevBeginIO( ASMR(a1) struct IOSana2Req *ioreq        ASMREG(a1),
                            ASMR(a6) DEVBASEP                        ASMREG(a6) )
{
	ULONG unit = (ULONG)ioreq->ios2_Req.io_Unit;

	ioreq->ios2_Req.io_Message.mn_Node.ln_Type = NT_MESSAGE;
	/*ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
	ioreq->ios2_WireError = S2WERR_GENERIC_ERROR;*/

	D(("BeginIO command %ld unit %ld\n",(LONG)ioreq->ios2_Req.io_Command,unit));

	switch( ioreq->ios2_Req.io_Command )
	{
		case CMD_READ:
			if( !ioreq->ios2_BufferManagement )
			{
				ioreq->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
				ioreq->ios2_WireError = S2WERR_BUFF_ERROR;
			}
			else
			{
			 if( !(db->db_Units[unit].du_Flags & DUF_ONLINE ))
			 {
			     ioreq->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
			     ioreq->ios2_WireError = S2WERR_UNIT_OFFLINE;
			 }
			 else
			 {
			     ioreq->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
			     ObtainSemaphore(&db->db_Units[unit].du_Sem);
			     dbAddReadReq( db, unit, ioreq );
			     ReleaseSemaphore(&db->db_Units[unit].du_Sem);
			     ioreq = (0);
			 }
			}
			break;

			/* Note: Orphans are not distributed to all listeners.
			         Strictly speaking, nobody actually asked for these
			         frame types, anyway. */
		case S2_READORPHAN:
			if( !ioreq->ios2_BufferManagement )
			{
				ioreq->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
				ioreq->ios2_WireError = S2WERR_BUFF_ERROR;
			}
			else
			{
			 if( !(db->db_Units[unit].du_Flags & DUF_ONLINE ))
			 {
			     ioreq->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
			     ioreq->ios2_WireError = S2WERR_UNIT_OFFLINE;
			 }
			 else
			 {
			     ioreq->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
			     ObtainSemaphore(&db->db_Units[unit].du_Sem);
			     ADDTAIL((struct List*)&db->db_Units[unit].du_ReadOrphans,(struct Node*)ioreq);
			     ReleaseSemaphore(&db->db_Units[unit].du_Sem);
			     ioreq = (0);
			 }
			}
			break;

		/* same routine for BC, MC, UC */
		case S2_BROADCAST:
			{
				UWORD *ptr = (UWORD*)ioreq->ios2_DstAddr;
				ptr[0] = ptr[1] = ptr[2] = 0xffff;
			}
		case S2_MULTICAST:
		case CMD_WRITE:
			{
			 ULONG mtu = db->db_Units[unit].du_MTU;
			 
			 if(ioreq->ios2_Req.io_Flags & SANA2IOF_RAW)
				mtu += 18; /* DEST MAC, SRC MAC, Type/Length, CRC */
			 if(ioreq->ios2_DataLength > mtu)
				ioreq->ios2_Req.io_Error = S2ERR_MTU_EXCEEDED;
			 else
			 {
			  if( !ioreq->ios2_BufferManagement )
			  {
				ioreq->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
				ioreq->ios2_WireError = S2WERR_BUFF_ERROR;
			  }
			  else
			  {
			   if( !(db->db_Units[unit].du_Flags & DUF_ONLINE ))
			   {
				ioreq->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
				ioreq->ios2_WireError = S2WERR_UNIT_OFFLINE;
			   }
			   else
			   {
				ioreq->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
				ObtainSemaphore(&db->db_Units[unit].du_Sem);
				 ADDTAIL((struct List*)&db->db_Units[unit].du_WriteQueue,(struct Node*)ioreq);
				ReleaseSemaphore(&db->db_Units[unit].du_Sem);
				Signal( (struct Task*)db->db_ServerProc, SIGBREAKF_CTRL_F );
				ioreq = (0);
			   }
			  }
			 }
			}
			break;

		case S2_ONEVENT:
			if (((ioreq->ios2_WireError&S2EVENT_ONLINE)&&(db->db_Units[unit].du_Flags&DUF_ONLINE)) ||
			    ((ioreq->ios2_WireError&S2EVENT_OFFLINE)&&!(db->db_Units[unit].du_Flags&DUF_ONLINE)))
			{
				ioreq->ios2_Req.io_Error = 0;
				ioreq->ios2_WireError &= (S2EVENT_ONLINE|S2EVENT_OFFLINE);
				dbTermIO(db,ioreq);
				ioreq = (0);
			}
			else if ((ioreq->ios2_WireError & (S2EVENT_ERROR|S2EVENT_TX|S2EVENT_RX|S2EVENT_ONLINE|
				  S2EVENT_OFFLINE|S2EVENT_BUFF|S2EVENT_HARDWARE|S2EVENT_SOFTWARE))
			         != ioreq->ios2_WireError)
			{
				ioreq->ios2_Req.io_Error = S2ERR_NOT_SUPPORTED;
				ioreq->ios2_WireError = S2WERR_BAD_EVENT;
			}
			else
			{
				/* handle request in Server task  */
				ioreq->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
				ObtainSemaphore(&db->db_Units[unit].du_Sem);
				AddTail((struct List*)&db->db_Units[unit].du_EventQueue, (struct Node*)ioreq);
				ReleaseSemaphore(&db->db_Units[unit].du_Sem);
				Signal((struct Task*)db->db_ServerProc, SIGBREAKF_CTRL_F );
				ioreq = (0);
			}
			break;

		case S2_ONLINE:
		case S2_OFFLINE:
		case S2_CONFIGINTERFACE:
			dbForwardIO( db, ioreq );
			ioreq = (0);
			break;

		case S2_GETSTATIONADDRESS:
			D(("CFGAddr0 %lx %lx %lx...\n",\
			   (LONG)db->db_Units[unit].du_CFGAddr[0],\
			   (LONG)db->db_Units[unit].du_CFGAddr[1],\
			   (LONG)db->db_Units[unit].du_CFGAddr[2] )); 

			CopyMem( db->db_Units[unit].du_CFGAddr, ioreq->ios2_SrcAddr, 6);
			CopyMem( db->db_Units[unit].du_HWAddr,  ioreq->ios2_DstAddr, 6);

			D(("CFGAddr1 %lx %lx %lx...\n",(LONG)*(ioreq->ios2_SrcAddr), (LONG)*(ioreq->ios2_SrcAddr+1), (LONG)*(ioreq->ios2_SrcAddr+2)));

			ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
			ioreq->ios2_WireError    = S2WERR_GENERIC_ERROR;
			break;

		case S2_DEVICEQUERY:
			{
			 struct Sana2DeviceQuery *query = ioreq->ios2_StatData;
			 
			 query->DevQueryFormat = 0;
			 query->DeviceLevel = 0;

			 D(("DeviceQuery size %ld\n",query->SizeAvailable));

			 if(query->SizeAvailable >= 18) query->AddrFieldSize = 48;
			 if(query->SizeAvailable >= 22) query->MTU           = db->db_Units[unit].du_MTU;
		         if(query->SizeAvailable >= 26) query->BPS           = db->db_Units[unit].du_BitPerSec;
			 if(query->SizeAvailable >= 30) query->HardwareType  = S2WireType_Ethernet;
			 if(query->SizeAvailable >= 34)
			 { /* RawMTU */
				ULONG *tmp = (ULONG*)&query->HardwareType;
				*(tmp+1) = 1518; /* DMAC,SMAC,TYPE,1500,CRC32 */
			 }
			 query->SizeSupplied = min( (ULONG)query->SizeAvailable, 34);
			 ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
			 ioreq->ios2_WireError    = S2WERR_GENERIC_ERROR;
			}
			break;

		case S2_TRACKTYPE:
			if( !dbTrackType( db, unit, ioreq->ios2_PacketType, 1 ))
				ioreq->ios2_Req.io_Error = S2ERR_NO_RESOURCES;
			else	ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
			ioreq->ios2_WireError    = S2WERR_GENERIC_ERROR;
			break;
		case S2_UNTRACKTYPE:
			if( !dbTrackType( db, unit, ioreq->ios2_PacketType, 0 ))
			{
		        	ioreq->ios2_Req.io_Error = S2ERR_BAD_STATE;
	        	        ioreq->ios2_WireError    = S2WERR_NOT_TRACKED;
			}
			else	
			{
				ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
				ioreq->ios2_WireError    = S2WERR_GENERIC_ERROR;
			}
			break;

		case S2_GETTYPESTATS:
			if( !dbTrackTypeStats( db, unit, ioreq->ios2_PacketType, (struct Sana2PacketTypeStats *)ioreq->ios2_StatData ))
			{
		        	ioreq->ios2_Req.io_Error = S2ERR_BAD_STATE;
	        	        ioreq->ios2_WireError    = S2WERR_NOT_TRACKED;
			}
			else	
			{
				ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
				ioreq->ios2_WireError    = S2WERR_GENERIC_ERROR;
			}
			break;

		case S2_GETGLOBALSTATS:
			CopyMem( &db->db_Units[unit].du_DevStats, ioreq->ios2_StatData, sizeof(struct Sana2DeviceStats) );
			ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
			ioreq->ios2_WireError    = S2WERR_GENERIC_ERROR;
			break;
#ifndef NSCMD_DEVICEQUERY
#warning "Building without NSD support"
#else
		case NSCMD_DEVICEQUERY:
			{
			 struct IOStdReq *stdrq = (struct IOStdReq *)ioreq;

			 D(("NSCMD_DEVICEQUERY size %ld\n",(ULONG)stdrq->io_Length));

			 if( (!stdrq->io_Data) || (stdrq->io_Length < sizeof( NSDQueryAnswer ) ) )
			 {
				ioreq->ios2_Req.io_Error = IOERR_BADLENGTH;
			 }
			 else
			 {
				CopyMem( (void*)&NSDQueryAnswer, stdrq->io_Data, sizeof( NSDQueryAnswer ) );
				stdrq->io_Actual = sizeof( NSDQueryAnswer ); 
				ioreq->ios2_Req.io_Error = 0;
			 }
			 D(("NSCMD_DEVICEQUERY return %ld\n",(ULONG)ioreq->ios2_Req.io_Error));
			}
			break;
		/* query contents may ask for live data: handle in server.c */
		case S2_DEVICEQUERYEXT:
			dbForwardIO( db, ioreq );
			ioreq = (0);
			break;
#endif

		case CMD_RESET:
		case CMD_UPDATE:
		case CMD_CLEAR:
		case CMD_STOP:
		case CMD_START:
		case CMD_FLUSH:
			ioreq->ios2_Req.io_Error = IOERR_NOCMD;
			/* we might not get a SANA-II request at this point, don't assume anything about struct */
			/* ioreq->ios2_WireError = S2WERR_GENERIC_ERROR; */
			break;

		case S2_ADDMULTICASTADDRESS:
		case S2_DELMULTICASTADDRESS:
#if 1
			dbForwardIO( db, ioreq );
			ioreq = (0);
#else
			ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
			ioreq->ios2_WireError    = S2WERR_GENERIC_ERROR;
#endif
			break;

		/*case S2_GETEXTENDEDGLOBALSTATS: */ /* S2R4 */
		case S2_GETSPECIALSTATS:
		default:
			ioreq->ios2_Req.io_Error = S2ERR_NOT_SUPPORTED;
			ioreq->ios2_WireError = S2WERR_GENERIC_ERROR;
			break;
	}

	if( ioreq )
		dbTermIO( db, ioreq );
}


ASM SAVEDS LONG DevAbortIO( ASMR(a1) struct IOSana2Req *ioreq        ASMREG(a1),
                            ASMR(a6) DEVBASEP                        ASMREG(a6) )
{
	/* we store ioreqs in various places, search them all */
	struct SanaReadType *type;
	struct SanaReader   *rd;
	LONG   ret = 0;

	ULONG unit = (ULONG)ioreq->ios2_Req.io_Unit;

	ObtainSemaphore(&db->db_Units[unit].du_Sem);

	  /* start with the read list -> types -> readers -> reqlist */
	  type = (struct SanaReadType *)GetHead( &db->db_Units[unit].du_ReadTypes );
	  while( type )
	  {
		rd = (struct SanaReader *)GetHead( &type->srt_Readers );
		while( rd )
		{
			if( dbIsInList( &rd->snr_Requests, (struct Node*)ioreq ))
				goto abort;

			rd = GetSucc( rd );
		}
		type = GetSucc( type );
	  }

	if( dbIsInList( (struct List*)&db->db_Units[unit].du_WriteQueue, (struct Node*)ioreq ))
		goto abort;
	if( dbIsInList( (struct List*)&db->db_Units[unit].du_EventQueue, (struct Node*)ioreq ))
		goto abort;
	if( dbIsInList( (struct List*)&db->db_Units[unit].du_ReadOrphans, (struct Node*)ioreq ))
		goto abort;

	ret = -1;
	goto end;
abort:
	Remove( (struct Node*)ioreq );
	ioreq->ios2_Req.io_Error = IOERR_ABORTED;
	ioreq->ios2_WireError = 0;
	ReplyMsg((struct Message*)ioreq);
end:
	ReleaseSemaphore(&db->db_Units[unit].du_Sem);
	return ret;
}


void dbTermIO( DEVBASEP, struct IOSana2Req *ioreq )
{
	D(("dbTermIO flags %ld\n",(LONG)ioreq->ios2_Req.io_Flags));

	if( ioreq->ios2_Req.io_Flags & SANA2IOF_QUICK )
		ioreq->ios2_Req.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
	else
		ReplyMsg( (struct Message *)ioreq );
}


/* private functions */
static void dbForwardIO( DEVBASEP, struct IOSana2Req *ioreq )
{
	ioreq->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
	PutMsg(db->db_ServerPort, (struct Message*)ioreq);

	D(("Forward Request %ld\n",ioreq->ios2_Req.io_Command));
}


static void dbNewList( struct List *l )
{
  l->lh_TailPred = (struct Node *)l;
  l->lh_Head = (struct Node *)&l->lh_Tail; 
  l->lh_Tail = 0;
}

static LONG dbIsInList( struct List *l, struct Node *entry )
{
 struct Node *nd;

 nd = GetHead( l );
 while( nd )
 {
	if( nd == entry ) 
		return 1;

	nd = GetSucc(nd);
 }

 return 0;
}


/* start Server function and keep it up */
/* note: server is global right now */
static LONG dbStartServer( DEVBASEP, LONG unit )
{
  struct MsgPort *port;
  LONG ret = 0,i;
  struct TagItem sv_tags[5];

  /* server running ? */
  if( db->db_ServerProc )
	return 1;
  
  if(!(port = CreateMsgPort() ))
  	return 0;

  sv_tags[i=0].ti_Tag  = NP_Entry;
  sv_tags[i++].ti_Data = (ULONG)ServerTask;
  sv_tags[i  ].ti_Tag  = NP_Name;
  sv_tags[i++].ti_Data = (ULONG)db->db_Lib.lib_Node.ln_Name;
  sv_tags[i  ].ti_Tag  = NP_Priority;
  sv_tags[i++].ti_Data = (ULONG)0;
  sv_tags[i  ].ti_Tag  = TAG_DONE;

  if( (db->db_ServerProc = CreateNewProc(sv_tags)))
  {
	/* server is running, wait for response */
	volatile struct ServerMsg msg;

	msg.sm_devbase = db;
	msg.sm_result  = 0;
	msg.sm_msg.mn_Length = sizeof(msg);
	msg.sm_msg.mn_ReplyPort = port;
	PutMsg( (struct MsgPort*)&db->db_ServerProc->pr_MsgPort, (struct Message*)&msg.sm_msg );
	WaitPort( port );
	ret = msg.sm_result; /* should be 1 */
	D(("Server answered with %ld\n",ret));
  }
  if( !ret )
  	db->db_ServerProc = (0);
  
  DeleteMsgPort( port );  

  return ret;
}


/* stop Server function */
/* note: server is global right now */
static LONG dbStopServer( DEVBASEP, LONG unit )
{
	ULONG i=1;
	volatile struct Task *P;

	/* server deletes itself from devbase */
	while( (P=(volatile struct Task*)db->db_ServerProc) )
	{
		Signal( (struct Task*)P, SIGBREAKF_CTRL_C);
		Delay(i);
		i = (i & 0xf) + 1;
	}

	return 0;
}


/* 
  delete all buffers (type references) associated with device opener
*/
static void dbDeleteBuffers( DEVBASEP, struct IOSana2Req *ioreq )
{
  struct db_BufferManagement *dbm;
  ULONG ID = 0;

  for( dbm    = (struct db_BufferManagement *)db->db_BufferManagement.lh_Head;
       (dbm->dbm_n.mln_Succ) && (ioreq->ios2_BufferManagement != (VOID*)dbm);
       dbm    = (struct db_BufferManagement *)dbm->dbm_n.mln_Succ ) {}

  /* delete BufferManagement structure if it was allocated by us (should be) */
  if( dbm->dbm_n.mln_Succ )
  {
	ID = dbm->dbm_ID;
  	Forbid();
	REMOVE( dbm );
	Permit();
	FreeMem( dbm, sizeof(struct db_BufferManagement) );
	ioreq->ios2_BufferManagement = (0);
  }

  /* scan all buffers of current unit for opener ID and discard associated data */
  if( ID )
  {
	ULONG  unit = (ULONG)ioreq->ios2_Req.io_Unit;

	dbDeleteReader( db, unit, ID );
  }
  /* we assume that no requests were left in the queues -> TODO: add some debug info to verify */

}


/*
  before calling:
   - get Semaphore
   - check BufferManagement
   - make sure ioreq is != NULL
*/
static void dbAddReadReq( DEVBASEP, ULONG unit, struct IOSana2Req *ioreq )
{
	struct SanaReadType *type;
	struct SanaReader *rd;
	ULONG frametype = ioreq->ios2_PacketType;
	ULONG ID = ((struct db_BufferManagement*)ioreq->ios2_BufferManagement)->dbm_ID;

	/* find type or add it (typically 2 types present: IP,ARP) */
	type = (struct SanaReadType *)db->db_Units[unit].du_ReadTypes.lh_Head;
	while( (type->srt_Node.mln_Succ) )
	{
		if( type->srt_Type == frametype )
			goto havetype;
	
		type = (struct SanaReadType *)type->srt_Node.mln_Succ;
	}
#if 0
	type = (struct SanaReadType *)GetHead( &db->db_Units[unit].du_ReadTypes );
	while( type )
	{
		if( type->srt_Type == frametype )
			goto havetype;
		type = GetSucc( type );
	}
#endif
	if( !(type = dbAddType( db, unit, frametype ) ))
		return; /* bail out, no mem left */
havetype:

	/* traverse readers on type (usually 1 reader) */
        rd = (struct SanaReader *)GetHead( &type->srt_Readers );
        while( rd )
        {
                if( rd->snr_ID == ID )
			goto havereader;
                rd = GetSucc(rd);
	}
	if( !(rd = dbAddReader(db,unit,frametype,type,ID ) ))
		return; /* bail out, no mem left */
havereader:

	ADDTAIL( &rd->snr_Requests , (struct Node*)ioreq );

}


/* add reader for frame type or directly to a type's list */
static struct SanaReader *dbAddReader( DEVBASEP, ULONG unit, ULONG frametype, struct SanaReadType *type, ULONG ID )
{
	struct SanaReader *rd;

	if( !type ) /* search types for ID if not explicitly given */
	{
		type = (struct SanaReadType *)GetHead( &db->db_Units[unit].du_ReadTypes );
		while( type )
		{
			if( type->srt_Type == frametype )
				break;
			type = GetSucc( type );
		}
	
		if( !type )
			return (0);	/* error: no such type */
	}
	
	rd = (struct SanaReader *)GetHead( &type->srt_Readers );
	while( rd )
	{
		if( rd->snr_ID == ID )
		{
			return rd;
		}
		rd = GetSucc(rd);
	}

	rd = (struct SanaReader *)AllocMem( sizeof( struct SanaReader ), MEMF_PUBLIC|MEMF_CLEAR );
	if( !rd )
		return (0);

	rd->snr_ID = ID;
	dbNewList( &rd->snr_Requests );

	if( (GetHead( &type->srt_Readers ) ))
		type->srt_Flags |= SRTF_MULTREADER;
	else	type->srt_Flags |= SRTF_ONEREADER;

	AddTail( &type->srt_Readers, (struct Node*)rd );

	return rd;
}


/* delete all references to reader among all types of specified unit */
static void dbDeleteReader( DEVBASEP, ULONG unit, ULONG ID )
{
	/* traverse types */
	struct SanaReadType *type,*t2;
	struct SanaReader *rd;

	type = (struct SanaReadType*)GetHead( &db->db_Units[unit].du_ReadTypes );
	while( type )
	{
		rd = (struct SanaReader *)GetHead( &type->srt_Readers );
		while( rd )
		{
			if( rd->snr_ID == ID )
			{
				REMOVE(rd);
				FreeMem(rd, sizeof( struct SanaReader ) );

				if( (t2 = GetHead( &type->srt_Readers ) ) )
				{
					if( !(GetSucc(t2) ))
						t2->srt_Flags &= ~SRTF_MULTREADER;
				}
				else	t2->srt_Flags &= ~(SRTF_ONEREADER|SRTF_MULTREADER);

				break;
			}
			rd = GetSucc(rd);
		}
		type = GetSucc( type );
	}
}


/* 
  add frame type to Read Queue
  
  note: the lifetime of read types is "infinite" right now, the
        type queues are deleted only on Device Expunge
*/
static struct SanaReadType *dbAddType( DEVBASEP, ULONG unit, ULONG frametype )
{
	struct SanaReadType *type;

	D(("dbAddType %ld to unit %ld\n",frametype,unit));

	/* check whether type is existing */
	type = (struct SanaReadType*)GetHead( &db->db_Units[unit].du_ReadTypes );
	while( type )
	{
		if( type->srt_Type == frametype ) /* type present: do nothing */
			return type;

		type = GetSucc( type );
	}

	/* */ 
	type = (struct SanaReadType*)AllocMem( sizeof( struct SanaReadType ), MEMF_PUBLIC|MEMF_CLEAR );
	if( type )
	{
		D(("dbAddType new type %ld to unit %ld\n",frametype,unit));
		type->srt_Type  = frametype;
		dbNewList( &type->srt_Readers );
		AddTail( (struct List*)&db->db_Units[unit].du_ReadTypes, (struct Node*)type );
	}
	return type;
}


/*
  Delete Type and readers on that type
*/
static void dbDeleteType( DEVBASEP, ULONG unit, ULONG frametype )
{
	struct SanaReadType *type;
	struct SanaReader *rd;

	type = (struct SanaReadType*)GetHead( &db->db_Units[unit].du_ReadTypes );
	while( type )
	{
		if( type->srt_Type == frametype ) 
			break;

		type = GetSucc( type );
	}

	if( !type )
		return;

	while( (rd = GetHead( &type->srt_Readers ) ))
	{
		REMOVE(rd);
		FreeMem(rd, sizeof( struct SanaReader ) );
	}

	REMOVE(type);
	FreeMem( type, sizeof( struct SanaReadType ) );
}


/* do track type statistics (1) or toggle (0) */
static LONG dbTrackType( DEVBASEP, ULONG unit, ULONG frametype, ULONG onoff )
{
	struct SanaReadType *type = dbAddType( db, unit, frametype );
	LONG   ret = 1;
	ULONG  flg = 0;

	if( !type )
		return 0;

	if( onoff )
	{
		type->srt_Flags |= SRTF_TRACKTYPE;
		db->db_Units[unit].du_Flags |= DUF_TYPETRACK;
		return ret;
	}

	if( !(type->srt_Flags & SRTF_TRACKTYPE ) )
		ret = 0;
	else
		type->srt_Flags ^= SRTF_TRACKTYPE;

	/* set TYPETRACK flag if any type on unit is still tracked           */
	/* well, actually this breaks if more than one stack is active. Duh. */
	type = (struct SanaReadType *)GetHead( &db->db_Units[unit].du_ReadTypes );
	while( type )
	{
		if( type->srt_Flags & SRTF_TRACKTYPE )
			flg = DUF_TYPETRACK;
		type = GetSucc( type );
	}
	db->db_Units[unit].du_Flags &= ~DUF_TYPETRACK;
	db->db_Units[unit].du_Flags |= flg;
		
	return ret;
}


/* return type statistics */
static LONG dbTrackTypeStats( DEVBASEP, ULONG unit, ULONG frametype, struct Sana2PacketTypeStats *statptr )
{
	struct SanaReadType *type = dbAddType( db, unit, frametype );
	if( !type )
		return 0;
	if( !(type->srt_Flags & SRTF_TRACKTYPE ) )
		return 0;

	CopyMem( &type->srt_Sana2PacketTypeStats, statptr, sizeof(struct Sana2PacketTypeStats) );
	return 1;
}

