;
;   device.i
;
;   (C) 2018 Henryk Richter <henryk.richter@gmx.net>
;
;   Device Functions and Definitions
;
;Note(1): structures are defined only partially at the moment
;Note(2): keep this in sync with device.h
;
        ifnd    _INC_DEVICE_I
_INC_DEVICE_I   EQU     1


; defaults
MAX_UNITS       EQU     4
DEF_BPS         EQU     10000000
DEF_MTU         EQU     1500

        ; includes
        include "exec/lists.i"
        include "exec/libraries.i"
        include "exec/devices.i"
	include "exec/semaphores.i"
	include "devices/timer.i"
        include "hw.i"

;
; this would belong in sana2.i
;
	STRUCTURE	Sana2DeviceStats,0
	ULONG          sds_PacketsReceived	;received count 
        ULONG          sds_PacketsSent	;transmitted count 
	ULONG          sds_BadData	;bad packets received 
	ULONG          sds_Overruns	;hardware miss count 
	ULONG          sds_Unused	;Unused field 
	ULONG          sds_UnknownTypesReceived	;orphan count 
	ULONG          sds_Reconfigurations	;network reconfigurations 
	STRUCT	       sds_LastStart,TV_SIZE ;
	LABEL	sds_SIZEOF

;
;
;
        STRUCTURE ServerData,0
        ULONG   sv_flags
        APTR    sv_frame                ;temporary frame for rx/tx
        ULONG   sv_framesize            ;allocated bytes
        STRUCT  sv_mclist,LH_SIZE       ;list of multicast addresses to accept
        LABEL   sv_SIZEOF

;
; Unit flags
;
DUB_OPEN	EQU	0
DUB_EXCLUSIVE	EQU	1
DUB_ONLINE	EQU	2
DUB_TYPETRACK	EQU	3
DUB_PROMISC	EQU	4
DUF_OPEN	EQU	(1<<DUB_OPEN)
DUF_EXCLUSIVE	EQU	(1<<DUB_EXCLUSIVE)
DUF_ONLINE	EQU	(1<<DUB_ONLINE)
DUF_TYPETRACK	EQU	(1<<DUB_TYPETRACK)
DUF_PROMISC	EQU	(1<<DUB_PROMISC)

;
;local to each unit of the device
;
        STRUCTURE       DevUnit,0
        STRUCT  du_ReadTypes,LH_SIZE       ;RX queues are sorted by type: find RX buffers 
                                           ;by type, afterwards traverse active readers = networking stacks
        STRUCT  du_ReadOrphans,LH_SIZE     ;not claimed by RX types or no buffers available -> orphan
        STRUCT  du_WriteQueue,LH_SIZE      ;TX queue 
        STRUCT  du_EventQueue,LH_SIZE      ;Event queue
        STRUCT  du_Sem,SS_SIZE             ;list locking (global per unit)

	ULONG	du_OpenCount		;unit openers
	ULONG	du_Flags		;unit flags
	ULONG	du_MTU			;maximum transmission unit (1500 for regular Ethernet)
	ULONG	du_BitPerSec		;speed in BPS
	STRUCT	du_HWAddr,6		;default MAC address
	STRUCT	du_CFGAddr,6		;configured MAC address

	STRUCT	du_DevStats,sds_SIZEOF	;collect device stats per unit

	; HW Data (generic for now) 
	ULONG	du_hwl0			;
	ULONG	du_hwl1			;
	ULONG	du_hwl2			;
	APTR	du_hwp0			;
	APTR	du_hwp1			;
	APTR	du_hwp2			;

        LABEL   du_SIZEOF


;
; Device Base
;
	STRUCT	devbase,LIB_SIZE
	APTR	db_SegList	;from Device Init, actually a BPTR

	ULONG		 db_Flags	;misc 
	APTR		db_SysBase	;Exec Base 
        APTR		db_DOSBase	;
	APTR		db_UtilityBase	;
	ULONG		db_NBoards	;
	ULONG		db_ID		;device opener ID enumeration 

	STRUCT		db_Units,du_SIZEOF*MAX_UNITS ;don't bother with lists, who's gonna have more 
	                                             ;than 1 network card of the same type, anyway ?

	; server process data 
	APTR		db_ServerProc		;process of main queue server 
	APTR		db_Task			;server shutdown task address (non permanently available)
	ULONG		db_ServerSigMask	;shutdown signal
	APTR		db_ServerPort		;request forwarding (initialized by server)

	STRUCT 		db_BufferManagement,LH_SIZE ;remember allocated instances 

	;services for other instances (server task, hw.c)
	STRUCT		db_svdat,sv_SIZEOF	;private data for main server
	;from hwprivate.i
	STRUCT		db_hwdat,4		;TODO: implement hwprivate.i

	LABEL		db_SIZEOF


;TODO:
;struct db_BufferManagement 
;struct ServerMsg 
;struct SanaReader
;struct SanaReadType
;struct MCastEntry


        endc    ;_INC_DEVICE_I
