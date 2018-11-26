/*
   server.h

  (C) 2018 Henryk Richter <henryk.richter@gmx.net>

  Main Dispatcher defs and protos

*/
#ifndef _INC_SERVER_H
#define _INC_SERVER_H

#define ETH_MTU 1500
#define MINPRIORITY -120
#define MAXPRIORITY  120

#define SERR_OK		1
#define SERR_ERROR	0
#define SERR_BUFFER_ERROR	-1
#define SERR_TX_ERROR	-2
#define SERR_RX_ERROR	-3

#if (MAX_UNITS > 1)
#define BOARDLOOP( _a_ ) for _a_ 
#else
#define BOARDLOOP( _a_ )
#endif


static void MemSet( void *a, ULONG val, ULONG size );
static struct ServerMsg *server_startup(void);
static LONG server_init( DEVBASEP );
static void server_up( DEVBASEP , struct ServerMsg *msg, LONG updown );
static LONG server_shutdown( DEVBASEP );
static LONG server_Config( DEVBASEP );
static LONG server_writequeue( DEVBASEP, ULONG unit );
static LONG server_HandleS2Commands( DEVBASEP );
static LONG server_SendEvent( DEVBASEP, ULONG unit, ULONG evtmask );
LONG server_writeerror( DEVBASEP, ULONG unit, struct IOSana2Req *ioreq, LONG code );
#ifdef EXTERNAL_WRITE_FRAME
LONG write_frame( DEVBASEP, ULONG unit, struct IOSana2Req *ioreq );
#else
static LONG write_frame( DEVBASEP, ULONG unit, struct IOSana2Req *ioreq );
#endif

static LONG server_writequeue( DEVBASETYPE*, ULONG );

static LONG server_setOnline( DEVBASETYPE* , ULONG, ULONG );
static void server_setOffline( DEVBASETYPE* , ULONG, ULONG );
static void server_CloseDevice_offline( DEVBASETYPE * );

#define svTermIO(_a_,_b_) ReplyMsg( (struct Message*)_b_)

#define COPYMAC( _d_, _s_ ) \
{\
 USHORT *dst = (USHORT*)(_d_);\
 USHORT *src = (USHORT*)(_s_);\
 *dst++ = *src++;\
 *dst++ = *src++;\
 *dst   = *src;\
}

#endif /* _INC_SERVER_H */


