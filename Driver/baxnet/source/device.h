/*
  device.h

  (C) 2018 Henryk Richter <henryk.richter@gmx.net>

  Device Functions and Definitions


*/
#ifndef _INC_DEVICE_H
#define _INC_DEVICE_H


/* defaults */
#define MAX_UNITS 4
#define DEF_BPS   10000000
#define DEF_MTU   1500


/* includes */
#include "compiler.h"
#include <dos/dos.h>
#include <exec/lists.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/semaphores.h>
#include <devices/sana2.h>
#include "hw.h"
#include "debug.h"


/* reassign Library bases from global definitions to own struct */
#define SysBase      db->db_SysBase
#define DOSBase      db->db_DOSBase
#define UtilityBase  db->db_UtilityBase
//#define VampireBase  db->db_VampireBase

/* old sana2.h ? */
#ifndef S2_CopyToBuff32
#define S2_CopyToBuff32      (S2_Dummy +  6)
#endif

#ifndef S2_CopyFromBuff32
#define S2_CopyFromBuff32    (S2_Dummy +  7)
#endif

struct ServerData {
	ULONG	sv_flags;
	UBYTE  *sv_frame;       /* temporary frame for rx/tx */
	ULONG	sv_framesize;   /* allocated bytes */
	struct  List sv_mclist; /* list of multicast addresses to accept */
};


#define DUB_OPEN	0
#define DUB_EXCLUSIVE	1
#define DUB_ONLINE	2
#define DUB_TYPETRACK	3
#define DUB_PROMISC     4
#define DUF_OPEN	(1<<DUB_OPEN)
#define DUF_EXCLUSIVE	(1<<DUB_EXCLUSIVE)
#define DUF_ONLINE	(1<<DUB_ONLINE)
#define DUF_TYPETRACK	(1<<DUB_TYPETRACK)
#define DUF_PROMISC	(1<<DUB_PROMISC)


struct DevUnit {
	volatile struct List du_ReadTypes;   /* RX queues are sorted by type: find RX buffers 
	                                        by type, afterwards traverse active readers =
	                                        networking stacks */
	volatile struct List du_ReadOrphans; /* not claimed by RX types or no buffers available -> orphan */

        volatile struct List du_WriteQueue; /* TX queue */
        volatile struct List du_EventQueue; /* Event queue */

	struct SignalSemaphore du_Sem;      /* list locking (global per unit) */
	struct SignalSemaphore du_WrSem;    /* experimental: separate Sempaphore for write list */

	ULONG	du_OpenCount;		/* unit openers */
	ULONG	du_Flags;		/* unit flags */
	ULONG	du_MTU;			/* maximum transmission unit (1500 for regular Ethernet) */
	ULONG	du_BitPerSec;		/* speed in BPS */
	UBYTE	du_HWAddr[6];		/* default MAC address    */
	UBYTE	du_CFGAddr[6];		/* configured MAC address */

	struct Sana2DeviceStats du_DevStats; /* collect device stats per unit */

	/* HW Data (generic for now) */
	ULONG	du_hwl0;
	ULONG	du_hwl1;
	ULONG	du_hwl2;
	APTR	du_hwp0;
	APTR	du_hwp1;
	APTR	du_hwp2;
};


struct devbase {
	struct  Library db_Lib;
	BPTR 	db_SegList; /* from Device Init */

	ULONG		 db_Flags;   /* misc */
	struct Library	*db_SysBase; /* Exec Base */
        struct Library	*db_DOSBase;
	struct Library  *db_UtilityBase;
	ULONG		 db_NBoards;
	ULONG		 db_ID;      /* device opener ID enumeration */

	struct DevUnit	db_Units[MAX_UNITS]; /* don't bother with lists, who's gonna have more 
	                                        than 1 network card of the same type, anyway ? */

	/* server process data */
	volatile struct Process *db_ServerProc;  /* process of main queue server */
	struct Task     *db_Task;          /* server shutdown task address (non permanently available) */
	ULONG		 db_ServerSigMask; /* shutdown signal */
	struct MsgPort  *db_ServerPort;    /* request forwarding (initialized by server) */

	/* housekeeping */
	struct List 	db_BufferManagement; /* remember allocated instances */

	/* services for other instances (server task, hw.c) */
	struct ServerData db_svdat;        /* private data for main server        */
	struct HWData     db_hwdat;        /* private data for hardware interface */  
};


typedef LONG (* ASM BMCALL)( ASMR(a0) void * ASMREG(a0), ASMR(a1) void * ASMREG(a1), ASMR(d0) LONG ASMREG(d0) );

struct db_BufferManagement {
	struct MinNode dbm_n;
	BMCALL	dbm_CopyFromBuffer32;
	BMCALL	dbm_CopyToBuffer32;
	BMCALL	dbm_CopyFromBuffer;
	BMCALL	dbm_CopyToBuffer;
	BMCALL	dbm_CopyFromBufferDMA;  /* this buffering method requires additional support from lowlevel HW implementation */
	BMCALL	dbm_CopyToBufferDMA;    /* usage of this pointer requires additional checks */
	APTR	dbm_PacketFilter;	/* this is actually a Hook */
	ULONG	dbm_ID;
};

#ifndef DEVBASETYPE
#define DEVBASETYPE struct devbase
#endif
#ifndef DEVBASEP
#define DEVBASEP DEVBASETYPE *db
#endif

struct ServerMsg {
	struct Message sm_msg;
	DEVBASETYPE   *sm_devbase;
	LONG           sm_result;
};


/* frame type management:
   - every network stack who is interested in a specific type gets a "reader" assigned to
     that type. The ID tracked here is the same that is assigned on OpenDevice and stored
     in the buffer management 
   - 
*/
struct SanaReader 
{
	 struct MinNode	snr_Node;
	 ULONG		snr_ID;   /* assigned to any reader on Device open */
	 struct List	snr_Requests;
};


/*
  read list(s)
   - multiple readers supported (i.e. multiple active Networking stacks)
   - one read list per type (shouldn't be too many)
   - two major thoughts behind this
     - track type statistics without additional search
     - received frame copy to additional reader(s) without searching through
       their read request lists
     + avoid long searches for orphans (unexpected received frames)
*/
struct SanaReadType
{
	struct MinNode	srt_Node;
	ULONG		srt_Type;	/* Type ID - matches Ethernet Types */
	ULONG		srt_Flags;	/* Flags: multiple reader mode, do track type */

	struct List	srt_Readers;	/* active readers */

	/* SANA-II Type statistics */
	struct Sana2PacketTypeStats srt_Sana2PacketTypeStats;

	ULONG		srt_FramesSent;
	ULONG		srt_FramesRcvd;
	ULONG		srt_BytesSent;
	ULONG		srt_BytesRcvd;
	ULONG		srt_FramesDropped;
};

/* SanaReadList flags */
#define SRTB_ONEREADER	0	/* at least one reader present ? */
#define SRTB_MULTREADER	1	/* more than one reader */
#define SRTB_TRACKTYPE	2	/* perform track type statistics */
#define SRTF_ONEREADER  (1<<SRTB_ONEREADER)
#define SRTF_MULTREADER	(1<<SRTB_MULTREADER)
#define SRTF_TRACKTYPE	(1<<SRTB_TRACKTYPE)

/* server maintains list of multicast addresses */
struct MCastEntry
{
	struct MinNode mce_Node;
	UBYTE  mce_start[6];
	UBYTE  mce_stop[6];
	ULONG  mce_refcount; /* ok, this is poor. Proper would mean "track by caller". */
	                     /* But hey, how much multicast do you encounter on Amiga? */
};

/* PROTOS */

//ASM LONG LibNull( void );

ASM SAVEDS struct Device *DevInit(ASMR(d0) DEVBASEP                  ASMREG(d0), 
                                  ASMR(a0) BPTR seglist              ASMREG(a0), 
				  ASMR(a6) struct Library *_SysBase  ASMREG(a6) );

ASM SAVEDS LONG DevOpen( ASMR(a1) struct IOSana2Req *ios2            ASMREG(a1), 
                         ASMR(d0) ULONG unit                         ASMREG(d0), 
                         ASMR(d1) ULONG flags                        ASMREG(d1),
                         ASMR(a6) DEVBASEP                           ASMREG(a6) );

ASM SAVEDS BPTR DevClose(   ASMR(a1) struct IOSana2Req *ios2         ASMREG(a1),
                            ASMR(a6) DEVBASEP                        ASMREG(a6) );

ASM SAVEDS BPTR DevExpunge( ASMR(a6) DEVBASEP                        ASMREG(a6) );

ASM SAVEDS VOID DevBeginIO( ASMR(a1) struct IOSana2Req *ios2         ASMREG(a1),
                            ASMR(a6) DEVBASEP                        ASMREG(a6) );

ASM SAVEDS LONG DevAbortIO( ASMR(a1) struct IOSana2Req *ios2         ASMREG(a1),
                            ASMR(a6) DEVBASEP                        ASMREG(a6) );

void dbTermIO( DEVBASETYPE*, struct IOSana2Req * );

/* server.c */
VOID SAVEDS ServerTask(void);

/* private functions */
#ifdef DEVICE_MAIN

static void dbNewList( struct List * );
static LONG dbIsInList( struct List *, struct Node * );

static LONG dbStartServer( DEVBASETYPE*, LONG );
static LONG dbStopServer( DEVBASETYPE*, LONG );

static void dbDeleteBuffers( DEVBASETYPE*, struct IOSana2Req * );

static void dbAddReadReq( DEVBASETYPE*, ULONG , struct IOSana2Req * );

static struct SanaReader*dbAddReader( DEVBASETYPE*, ULONG, ULONG, struct SanaReadType *, ULONG );
static void dbDeleteReader( DEVBASETYPE*, ULONG, ULONG );

static struct SanaReadType *dbAddType( DEVBASETYPE*, ULONG, ULONG );
static void dbDeleteType( DEVBASETYPE*, ULONG, ULONG );

static void dbForwardIO( DEVBASETYPE*, struct IOSana2Req * );

static LONG dbTrackType(DEVBASETYPE*, ULONG, ULONG, ULONG );
static LONG dbTrackTypeStats( DEVBASETYPE*, ULONG, ULONG, struct Sana2PacketTypeStats *);
#endif /* DEVICE_MAIN */


#endif /* _INC_DEVICE_H */
