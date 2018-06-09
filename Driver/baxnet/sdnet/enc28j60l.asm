; ------------------------------------------------------------------------------
; | SPI Lowlevel Access to SD-Card slot of Vampire 500/600 cards               |
; | Henryk Richter <henryk.richter@gmx.net>                                    |
; ------------------------------------------------------------------------------
;
; Note: This code is in some places intentionally saving more registers than 
;       required by the AmigaOS ABI.
;

; enable debugging code block
DEBUG	EQU	0
;
_OPT_CFGSPEED_BYTE	EQU	0	;write SPI clock divider as byte (1) or word (0)
_OPT_WriteOp_Delay	EQU	0	;delay after writeOp (1) or no delay (0)
_OPT_READ_CONST		EQU	$ff	;output bits in read mode (default 0xff, for testing 0x00)
_OPT_SHORT_DELAY	EQU	1	;small delays by a single CIA read
;
;
; SPI Hardware dependent routines
;
;
	;lowlevel register read/write 
	XDEF	_enc28j60l_ReadOp
	XDEF	_enc28j60l_WriteOp

	;write buffer, read buffer
	XDEF	_enc28j60l_ReadBuffer
	XDEF	_enc28j60l_WriteBuffer

	; of these, only SPISpeed bangs the hardware
	XDEF	_enc28j60l_Init
	XDEF	_enc28j60l_Shutdown
	XDEF	_enc28j60l_SetSPISpeed

;
;
; hardware support functions without SPI register banging
; (moved into ASM source to have easy debugging in ASM-One)
;
;
;read/write one 8 Bit Register
	XDEF	_enc28j60l_ReadRegByte
	XDEF	_enc28j60l_WriteRegByte
;
	XDEF	_enc28j60l_SetBank

	;read/write one 16 Bit Register
	XDEF	_enc28j60l_ReadReg
	XDEF	_enc28j60l_WriteReg

	;delay min. D0 microseconds
	XDEF	_enc28j60l_UMinDelay

	;machine	68020

;	section	code,code

; Register definitions (used only in Debug mode so far)
;
ADDR_MASK   EQU     $1F
BANK_MASK   EQU     $60
SPRD_MASK   EQU     $80
BANK23_MASK EQU     $40

;// All-bank registers
EIE        EQU      $1B
EIR        EQU      $1C
ESTAT      EQU      $1D
ECON2      EQU      $1E
ECON1      EQU      $1F
;// Bank 0 registers
ERDPT      EQU      ($00|$00)
EWRPT      EQU      ($02|$00)
ETXST      EQU      ($04|$00)
ETXND      EQU      ($06|$00)
ERXST      EQU      ($08|$00)
ERXND      EQU      ($0A|$00)
ERXRDPT    EQU      ($0C|$00)
;// ERXWRPT         ($0E|$00)
EDMAST     EQU      ($10|$00)
EDMAND     EQU      ($12|$00)
;//  EDMADST         ($14|$00)
EDMACS     EQU      ($16|$00)
;// Bank 1 registers
EHT0       EQU      ($00|$20)
EHT1       EQU      ($01|$20)
EHT2       EQU      ($02|$20)
EHT3       EQU      ($03|$20)
EHT4       EQU      ($04|$20)
EHT5       EQU      ($05|$20)
EHT6       EQU      ($06|$20)
EHT7       EQU      ($07|$20)
EPMM0      EQU      ($08|$20)
EPMM1      EQU      ($09|$20)
EPMM2      EQU      ($0A|$20)
EPMM3      EQU      ($0B|$20)
EPMM4      EQU      ($0C|$20)
EPMM5      EQU      ($0D|$20)
EPMM6      EQU      ($0E|$20)
EPMM7      EQU      ($0F|$20)
EPMCS      EQU      ($10|$20)
EWOLIE     EQU      ($16|$20)
EWOLIR     EQU      ($17|$20)
ERXFCON    EQU      ($18|$20)
EPKTCNT    EQU      ($19|$20)
;// Bank 2 registers
MACON1     EQU      ($00|$40|$80)
MACON2     EQU      ($01|$40|$80)
MACON3     EQU      ($02|$40|$80)
MACON4     EQU      ($03|$40|$80)
MABBIPG    EQU      ($04|$40|$80)
MAIPG      EQU     ($06|$40|$80)
MACLCON1   EQU      ($08|$40|$80)
MACLCON2   EQU      ($09|$40|$80)
MAMXFL     EQU     ($0A|$40|$80)
MAPHSUP    EQU      ($0D|$40|$80)
MICON      EQU      ($11|$40|$80)
MICMD      EQU      ($12|$40|$80)
MIREGADR   EQU      ($14|$40|$80)
MIWR       EQU     ($16|$40|$80)
MIRD       EQU     ($18|$40|$80)
;// Bank 3 registers
MAADR1     EQU      ($00|$60|$80)
MAADR0     EQU      ($01|$60|$80)
MAADR3     EQU      ($02|$60|$80)
MAADR2     EQU      ($03|$60|$80)
MAADR5     EQU      ($04|$60|$80)
MAADR4     EQU      ($05|$60|$80)
EBSTSD     EQU      ($06|$60)
EBSTCON    EQU      ($07|$60)
EBSTCS     EQU     ($08|$60)
MISTAT     EQU      ($0A|$60|$80)
EREVID     EQU      ($12|$60)
ECOCON     EQU      ($15|$60)
EFLOCON    EQU      ($17|$60)
EPAUS      EQU     ($18|$60)

BANK23REG	EQU	$40	;used to check for MII and MAC registers

	ifne	0

;// ENC28J60 ERXFCON Register Bit Definitions
ERXFCON_UCEN     $80
ERXFCON_ANDOR    $40
ERXFCON_CRCEN    $20
ERXFCON_PMEN     $10
ERXFCON_MPEN     $08
ERXFCON_HTEN     $04
ERXFCON_MCEN     $02
ERXFCON_BCEN     $01
;// ENC28J60 EIE Register Bit Definitions
EIE_INTIE        $80
EIE_PKTIE        $40
EIE_DMAIE        $20
EIE_LINKIE       $10
EIE_TXIE         $08
EIE_WOLIE        $04
EIE_TXERIE       $02
EIE_RXERIE       $01
;// ENC28J60 EIR Register Bit Definitions
EIR_PKTIF        $40
EIR_DMAIF        $20
EIR_LINKIF       $10
EIR_TXIF         $08
EIR_WOLIF        $04
EIR_TXERIF       $02
EIR_RXERIF       $01

	endc

;// ENC28J60 ESTAT Register Bit Definitions
ESTAT_INT      EQU   $80
ESTAT_LATECOL  EQU   $10
ESTAT_RXBUSY   EQU   $04
ESTAT_TXABRT   EQU   $02
ESTAT_CLKRDY   EQU   $01
;// ENC28J60 ECON2 Register Bit Definitions
ECON2_AUTOINC  EQU   $80
ECON2_PKTDEC   EQU   $40
ECON2_PWRSV    EQU   $20
ECON2_VRPS     EQU   $08
;// ENC28J60 ECON1 Register Bit Definitions
ECON1_TXRST    EQU  $80
ECON1_RXRST    EQU  $40
ECON1_DMAST    EQU  $20
ECON1_CSUMEN   EQU  $10
ECON1_TXRTS    EQU  $08
ECON1_RXEN     EQU  $04
ECON1_BSEL1    EQU  $02
ECON1_BSEL0    EQU  $01

	ifne	0
;// ENC28J60 MACON1 Register Bit Definitions
MACON1_LOOPBK    $10
MACON1_TXPAUS    $08
MACON1_RXPAUS    $04
MACON1_PASSALL   $02
MACON1_MARXEN    $01
;// ENC28J60 MACON2 Register Bit Definitions
MACON2_MARST     $80
MACON2_RNDRST    $40
MACON2_MARXRST   $08
MACON2_RFUNRST   $04
MACON2_MATXRST   $02
MACON2_TFUNRST   $01
;// ENC28J60 MACON3 Register Bit Definitions
MACON3_PADCFG2   $80
MACON3_PADCFG1   $40
MACON3_PADCFG0   $20
MACON3_TXCRCEN   $10
MACON3_PHDRLEN   $08
MACON3_HFRMLEN   $04
MACON3_FRMLNEN   $02
MACON3_FULDPX    $01
;// ENC28J60 MICMD Register Bit Definitions
MICMD_MIISCAN    $02
MICMD_MIIRD      $01
;// ENC28J60 MISTAT Register Bit Definitions
MISTAT_NVALID    $04
MISTAT_SCAN      $02
MISTAT_BUSY      $01

;// ENC28J60 EBSTCON Register Bit Definitions
EBSTCON_PSV2     $80
EBSTCON_PSV1     $40
EBSTCON_PSV0     $20
EBSTCON_PSEL     $10
EBSTCON_TMSEL1   $08
EBSTCON_TMSEL0   $04
EBSTCON_TME      $02
EBSTCON_BISTST    $01

;// PHY registers
PHCON1           $00
PHSTAT1          $01
PHHID1           $02
PHHID2           $03
PHCON2           $10
PHSTAT2          $11
PHIE             $12
PHIR             $13
PHLCON           $14

;// ENC28J60 PHY PHCON1 Register Bit Definitions
PHCON1_PRST      $8000
PHCON1_PLOOPBK   $4000
PHCON1_PPWRSV    $0800
PHCON1_PDPXMD    $0100
;// ENC28J60 PHY PHSTAT1 Register Bit Definitions
PHSTAT1_PFDPX    $1000
PHSTAT1_PHDPX    $0800
PHSTAT1_LLSTAT   $0004
PHSTAT1_JBSTAT   $0002
;// ENC28J60 PHY PHCON2 Register Bit Definitions
PHCON2_FRCLINK   $4000
PHCON2_TXDIS     $2000
PHCON2_JABBER    $0400
PHCON2_HDLDIS    $0100

;// ENC28J60 Packet Control Byte Bit Definitions
PKTCTRL_PHUGEEN  $08
PKTCTRL_PPADEN   $04
PKTCTRL_PCRCEN   $02
PKTCTRL_POVERRIDE $01


	endc

;// SPI operation codes
ENC28J60_READ_CTRL_REG   EQU    $00
ENC28J60_READ_BUF_MEM    EQU    $3A
ENC28J60_WRITE_CTRL_REG  EQU    $40
ENC28J60_WRITE_BUF_MEM   EQU    $7A
ENC28J60_BIT_FIELD_SET   EQU    $80
ENC28J60_BIT_FIELD_CLR   EQU    $A0
ENC28J60_SOFT_RESET      EQU    $FF

RXSTART_INIT   EQU     $0000  // start of RX buffer, room for 4 packets
RXSTOP_INIT    EQU     $19FF  // end of RX buffer

TXSTART_INIT   EQU     $1A00  // start of TX buffer, room for 1 packet
TXSTOP_INIT    EQU     $1FFF  // end of TX buffer

;// max frame length which the conroller will accept:
;// (note: maximum ethernet frame length would be 1518)
MAX_FRAMELEN   EQU     1518        


;Amiga custom registers
OCS_VHPOSR		EQU	$DFF006

;
;
;// Vampire SDCard Registers
;
;
	ifd	PROTO_V2EXPNET
SDCARD_DATA_W             EQU   $00DE0010
SDCARD_DATA_R             EQU   $00DE0010
SDCARD_CFG_W              EQU   $00DE0014
SDCARD_STATUS_R           EQU   $00DE0016
SDCARD_CFG_SPEED          EQU	$00DE001C
	else	;PROTO_V2EXPNET
	; PROTO_SDNET
SDCARD_DATA_W             EQU   $00DE0000
SDCARD_DATA_R             EQU   $00DE0000
SDCARD_CFG_W              EQU   $00DE0004
SDCARD_STATUS_R           EQU   $00DE0006
SDCARD_CFG_SPEED          EQU	$00DE000C
	endc	;PROTO_V2EXPNET

SDCARD_SPEED              EQU   $20		;this one is obsolete
SDCARD_CS                 EQU   $1
SDCARD_CSn                EQU   $0


CSDELAY	macro
	ifne	_OPT_SHORT_DELAY	
	tst.w	$DFF00A		;1 chipset access 560ns
        ;tst.b   $BFE301        ;1 CIA access = 1.4 us
	endc	
	endm


SPI_ENABLE	macro
		;lea.l	SDCARD_CFG_W,a0
		;move.w	#SDCARD_CSn+SDCARD_SPEED,(a0)
		move.w  #SDCARD_CSn+SDCARD_SPEED,SDCARD_CFG_W
		endm

SPI_DISABLE	macro
		;lea.l	SDCARD_CFG_W,a0
		;move.w	#SDCARD_CS+SDCARD_SPEED,(a0)
		move.w	#SDCARD_CS+SDCARD_SPEED,SDCARD_CFG_W
		endm


; ------------------------------------------------------------------------------
; |                                                                            |
; | Test/Debug code                                                            |
; |                                                                            |
; ------------------------------------------------------------------------------
	ifne 	DEBUG

ExecBase                 EQU    4  ; Exec.Base()
OpenLibrary              EQU -552  ; D0:libBase = Exec.OpenLibrary(A1:libName,D0:version)
CloseLibrary             EQU -414  ; Exec.CloseLibrary(A1:libBase)
PutStr                   EQU -948  ; DOS.PutStr(D1:str)
_LVODelay		EQU	-198

START:
opendos:
	move.l	ExecBase,a6
	lea			DosLibrary,a1
	moveq		#0,d0
	jsr			OpenLibrary(a6)
	tst.l		d0
	beq			error
	move.l	d0,a6

	moveq	#0,d7		;no error

;	bsr		cardCSn
;	bsr		cardCS
;	bsr		cardCSn

	;
	;_enc28j60l_WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	;
	move.b	#ENC28J60_SOFT_RESET,d3
	move.b	#0,d0
	move.b	#ENC28J60_SOFT_RESET,d1
	bsr	_enc28j60l_WriteOp

	;delay 1/50s = 20ms (2ms would be sufficient)
	moveq	#1,d1
	jsr	_LVODelay(a6)

	move	#$fff,d2

	move.b	#ENC28J60_READ_CTRL_REG,d3
	move.b	#ESTAT,d0
	bsr	_enc28j60l_ReadOp

	and.b	#ESTAT_CLKRDY,d0	; (=1)
	bne.s	.foundrdy
	dbf	d2,.loop
	or.b	#1,d7			;searchtest error
.foundrdy



	move.b	#ECON1,d0
	move.b	#4,d1
	bsr	_enc28j60l_WriteRegByte

	move.b	#MACON3,d0
	bsr	_enc28j60l_SetBank

	move.b	#ECON1,d0
	bsr	_enc28j60l_ReadRegByte
	


	moveq	#1,d0
	lea	$DE000C,A0		;SDCARD_CFG_SPEED
	move.w	d0,(a0)


	move	#$1ff,d6	;
.loop
	move.b	#MACON3,d0
	move.b	#48+1,d1
	bsr	_enc28j60l_WriteRegByte

	move.b	#ECON1,d0
	bsr	_enc28j60l_ReadRegByte

	move.b	#MACON1,d0
	move.b	#$d,d1
	bsr	_enc28j60l_WriteRegByte

	move.b	#MACON3,d0
	bsr	_enc28j60l_ReadRegByte
	cmp.b	#48+1,d0
	beq.s	.ok
	or.b	#2,d7			;MACON write error
.ok
	dbf	d6,.loop
	;btst	#6,$bfe001
	;bne.s	.loop


	move.b	#MACON1,d0
	move.b	#$d,d1
	bsr	_enc28j60l_WriteRegByte

	move.b	#MACON4,d0
	move.b	#64,d1
	bsr	_enc28j60l_WriteRegByte


	move.b	#MACON1,d0
	bsr	_enc28j60l_ReadRegByte
	cmp.b	#$ff,d0
	beq.s	.skip
	nop
.skip

	move.b	#EIR,d0
	bsr	_enc28j60l_ReadRegByte


	;  while (!readOp(ENC28J60_READ_CTRL_REG, ESTAT) & ESTAT_CLKRDY) {
	move.b	#ENC28J60_READ_CTRL_REG,d3
	move.b	#ESTAT,d0
	bsr	_enc28j60l_ReadOp


	move.b	#EREVID,d0
	bsr	_enc28j60l_ReadRegByte

	;bsr			carddetect
	;bsr			cardCS

	move.b	#MACON1,d0
	bsr	_enc28j60l_ReadRegByte


	move.b	#EWRPT,d0
	move.w	#TXSTART_INIT,d1
	bsr	_enc28j60l_WriteReg

	bsr	_enc28j60l_ReadReg
	cmp.w	d0,d1
	beq.s	.ewrptmatch
	or.b	#4,d7
.ewrptmatch


	move.b	#EWRPT,d0
	bsr	_enc28j60l_ReadRegByte
	move.b	#EWRPT+1,d0
	bsr	_enc28j60l_ReadRegByte



	;----------- write custom MAC address and compare afterwards ---------

	lea	mactest,a0
	moveq	#6-1,d4
.macwr
	move.b	(a0)+,d1
	move.b	-1+macdest-mactest(a0),d0
	bsr	_enc28j60l_WriteRegByte
	dbf	d4,.macwr

	lea	mactest,a0
	moveq	#6-1,d4
.macrd
	move.b	macdest-mactest(a0),d0
	addq.l	#1,a0
	bsr	_enc28j60l_ReadRegByte
	move.b	d0,-1+mactestr-mactest(a0)
	dbf	d4,.macrd

	lea	mactest,a0
	moveq	#6-1,d4
.macver
	move.b	(a0)+,d0
	cmp.b	-1+mactestr-mactest(a0),d0
	beq.s	.match
	or.b	#8,d7
.match
	dbf	d4,.macver


	;------------------ READBUFFER/WRITEBUFFER TEST -----------------------

	lea	mactest,a1
	move.w	#32,d0
	move.w	#TXSTART_INIT,d1
	movem.l	d0-d1/a0-a1,-(sp)
	bsr	_enc28j60l_WriteBuffer
	movem.l	(sp)+,d0-d1/a0-a1

	;Dest Ptr to HW
	moveq	#ERDPT,d0	
	move.w	#TXSTART_INIT+1,d1	;skip status byte
	bsr	_enc28j60l_WriteReg

	lea	readframe,a1
	move.w	#32,d0
	movem.l	d0-d1/a0-a1,-(sp)
	bsr	_enc28j60l_ReadBuffer
	movem.l	(sp)+,d0-d1/a0-a1

	rts

DosLibrary:		dc.b "dos.library",0
mactest:	dc.b	$00,$80,$10,$b,$a,$ff
mactestr:	dc.b	0,0,0,0,0,0
macdest:	dc.b	MAADR5,MAADR4,MAADR3,MAADR2,MAADR1,MAADR0
		dc.b	$de,$ad,$be,$ef,$fe,$ed,$fa,$ce
		dc.b	1,2,3,4,5,6
		
readframe:	ds.b	32

	endc
; ------------------------------------------------------------------------------
; |                                                                            |
; | END Test/Debug code                                                        |
; |                                                                            |
; ------------------------------------------------------------------------------




;------------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------------
;
; SPI HW dependent functions
;
;------------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------------


; Initialize SPI hardware
;
; Not much to be done right now, just initialize the SPI clock divider and that's it
;
; In: void 
;
; Out:
;  D0 <= 0 - failure
;        1 
_enc28j60l_Init:

	SPI_DISABLE

	moveq	#3,d0			;conservative default for Chip initialization
	bsr	_enc28j60l_SetSPISpeed	;keeps all regs

	SPI_DISABLE			;/CS = HIGH

	;moveq	#1,d0
	rts




; Shutdown SPI hardware
;
; In:  void
; Out: void
_enc28j60l_Shutdown:

	SPI_DISABLE			;/CS = HIGH

	rts




; Set SPI clock divider
;
; In:  D0 = 1...$1ff
; Out: void 
_enc28j60l_SetSPISpeed:
	move.l	d0,-(sp)

	cmp.w	#0,d0		;1
	bgt.s	speed_minok$	;
	moveq	#0,d0		;1
speed_minok$

	cmp.w	#$7f,d0		;we don't want to be too slow!
	blt.s	speed_maxok$
	move.w	#$7f,d0
speed_maxok$

	ifne	_OPT_CFGSPEED_BYTE
		move.b	d0,SDCARD_CFG_SPEED
	else
		move.w	d0,SDCARD_CFG_SPEED
	endc

	move.l	(sp)+,d0
	rts

; Read a number of Bytes from the ENC28J60
; note: ENC28J60 Read pointer assumed to be written already
;A1 = Ptr
;D0 = Size (>=2)
_enc28j60l_ReadBuffer:

	SPI_ENABLE
	movem.l	d3-d5,-(sp)
	;CSDELAY
	lea.l	SDCARD_DATA_W,a0	;SPI_OUT
	move.b	#ENC28J60_READ_BUF_MEM,(a0)
	move.b	#_OPT_READ_CONST,d4	;_OPT_READ_CONST
	move.b	(a0),d3	;wait		;write and read register are the same on Vampire

	move.b	d4,(a0)			;provide clock (0xff here)
	subq.l	#1,d0			;we start a second transfer in main loop
	bra.s	.loop_enter
.loop:
	move.b	d5,(a1)+
	subq.l	#1,d0
	ble.s	.loop_done		;finish last transfer outside
.loop_enter:
	move.b	(a0),d5
	move.b	d4,(a0)
	bra.s	.loop
.loop_done:
	move.b	(a0),(a1)+

	movem.l	(sp)+,d3-d5

	;CSDELAY
	SPI_DISABLE

	rts

	ifne	0
; Read a number of Bytes from the ENC28J60
; note: ENC28J60 Read pointer assumed to be written already
;A1 = Ptr
;D0 = Size
_enc28j60l_ReadBuffer_complicated:

	SPI_ENABLE
	movem.l	d3-d5,-(sp)
	;CSDELAY
	lea.l	SDCARD_DATA_W,a0	;SPI_OUT
	move.b	#ENC28J60_READ_BUF_MEM,(a0)
	moveq	#7,d1
	and.l	d0,d1
	lsr.l	#3,d0
	move.b	#_OPT_READ_CONST,d4	;_OPT_READ_CONST

	move.b	(a0),d3	;wait		;write and read register are the same on Vampire

	tst.l	d0
	beq.s	.nomain

	;CSDELAY

	ifne	1
	;----------------------- NEW --------------------------

	rept	4
	 move.b	d4,(a0)			;provide clock (0xff here)
	 lsl.l	#8,d5
	 move.b	(a0),d5
	endr
	bra.s	.loop_enter
.loop:
 	move.b	(a0),d5

	move.b	d4,(a0)			;provide clock (0xff here)
	move.l	d5,(a1)+
	move.b	(a0),d5

	rept	3
	 move.b	d4,(a0)			;provide clock (0xff here)
	 lsl.l	#8,d5
	 move.b	(a0),d5
	endr

.loop_enter:
	  move.b	d4,(a0)			;provide clock (0xff here)
	 move.l	d5,(a1)+	
	  move.b	(a0),d5

	rept	2
	  move.b	d4,(a0)			;provide clock (0xff here)
	  lsl.l		#8,d5
	  move.b	(a0),d5
	endr

	  move.b	d4,(a0)			;provide clock (0xff here)
	  lsl.l		#8,d5

	subq.l	#1,d0
	bne.s	.loop

 	move.b	(a0),d5
	move.l	d5,(a1)+

	;----------------------- NEW --------------------------
	else
	;----------------------- OLD --------------------------
.loop:
	rept	4
	 move.b	d4,(a0)			;provide clock (0xff here)
	 lsl.l	#8,d5
	 move.b	(a0),d5
	endr

	  move.b	d4,(a0)			;provide clock (0xff here)
	 move.l	d5,(a1)+	
	  move.b	(a0),d5

	rept	3
	  move.b	d4,(a0)			;provide clock (0xff here)
	  lsl.l		#8,d5
	  move.b	(a0),d5
	endr

	  move.l	d5,(a1)+
	  
	subq.l	#1,d0
	bne.s	.loop

	;----------------------- OLD --------------------------
	endc	

.nomain:
	tst.l	d1
.tail:
	beq.s	.notail
	 move.b	d4,(a0)			;provide clock (0xff here)
	 move.b	(a0),(a1)+		;read data being shifted in
	subq.l	#1,d1
	bra.s	.tail
.notail:

	movem.l	(sp)+,d3-d5

	;CSDELAY
	SPI_DISABLE

	rts
	endc

; Write a number of Bytes to the ENC28J60
;
;A1 = Ptr
;D0 = Size
;D1 = DestPtr on 28j60
_enc28j60l_WriteBuffer:
	movem.l	d3-d5,-(sp)

	;Dest Ptr to HW
	move.l	d0,d4		;save length
	moveq	#EWRPT,d0
	bsr	_enc28j60l_WriteReg

	;now one header byte via SPI and then the Data
	SPI_ENABLE

	CSDELAY

	;-> COMMAND
	lea.l	SDCARD_DATA_W,a0	;SPI_OUT
	move.b	#ENC28J60_WRITE_BUF_MEM,(a0)
	move.b	(a0),d0	;wait

	;-> 1st byte is control byte
	move.b	#$f,(a0) ;1=Override, 2=CRCGEN, 4=PADDING, 8=HUGE ENABLE

	ifne	1

.loopwb:
	move.b	(a1)+,d1
	move.b	(a0),d0
	
	move.b	d1,(a0)

	subq.l	#1,d4
	bne.s	.loopwb

	move.b	(a0),d0	;last wait

	else

	moveq	#7,d1	;
	and.l	d4,d1	;tail bytes
	lsr.l	#3,d4	;8 bytes written in one run

	move.b	(a0),d0  ;wait
	
loopwb$
	; 8 bytes in one run, avoiding subq/bne overhead
	move.b	(a1)+,(a0)
	move.b	(a0),d0	;wait
	move.b	(a1)+,(a0)
	move.b	(a0),d0	;wait
	move.b	(a1)+,(a0)
	move.b	(a0),d0	;wait
	move.b	(a1)+,(a0)
	move.b	(a0),d0	;wait
	move.b	(a1)+,(a0)
	move.b	(a0),d0	;wait
	move.b	(a1)+,(a0)
	move.b	(a0),d0	;wait
	move.b	(a1)+,(a0)
	move.b	(a0),d0	;wait
	move.b	(a1)+,(a0)
	move.b	(a0),d0	;wait

	subq.l	#1,d4
	bne.s	loopwb$

	; leftover bytes ?
	tst.l	d1
looptail$
	beq.s	notail$
	move.b	(a1)+,(a0)
	move.b	(a0),d0	;wait
	subq.l	#1,d1
	bra.s	looptail$
notail$

	endc


	movem.l	(sp)+,d3-d5		;restore registers before SPI_DISABLE

	; CSDELAY
	SPI_DISABLE

	rts



;static void _enc28j60l_WriteOp (uint8_t op, uint8_t address, uint8_t data) {
;    spi_enable_eth();
;    spi_out(op | (address & ADDR_MASK));
;    spi_out(data);
;    spi_disable_eth();
;}
; D0 - address
; D1 - data
; D3 - Op
_enc28j60l_WriteOp:
	movem.l	d0/d2/d3/a0,-(sp)

	SPI_ENABLE

	moveq	#BANK23_MASK,d2
	and.b	d0,d2

	and.b	#ADDR_MASK,d0
	or.b	d0,d3

	lea.l	SDCARD_DATA_W,a0	;SPI_OUT
	move.b	d3,(a0)
	move.b	(a0),d3	;wait

	;lea.l	SDCARD_DATA_W,a0	;SPI_OUT
	move.b	d1,(a0)
	move.b	(a0),d3 ;wait

	ifeq	_OPT_WriteOp_Delay
	 tst.b	d2			;MAC or MII-Register ?
	 beq.s	nodelay1$
	endc

	moveq	#10,d0	;wait some more to let the data sink in
noplop$
	CSDELAY
	dbf	d0,noplop$

nodelay1$

	SPI_DISABLE

	;move.b	$BFD400,d3		;stall a little to fulfull /CS high minimum requirements (50ns)
	nop
	nop

	ifne	0

	moveq	#60,d0	;wait some more to fulfill minimal /CS delays (this delay is clearly too much right now...)
noplop2$:
	nop
	dbf	d0,noplop2$

	endc

	movem.l	(sp)+,d0/d2/d3/a0
	rts


; D3 - Op
; D0 - Address
;
; ret: D0 - result
_enc28j60l_ReadOp:
	move.l	a0,-(sp)

	SPI_ENABLE

	move.l	d1,-(sp)

	move.b	#$80,d1			;address & 0x80
	and.b	d0,d1

	and.b	#ADDR_MASK,d0
	or.b	d3,d0

	lea.l	SDCARD_DATA_W,a0	;SPI_OUT
	move.b	d0,(a0)

	tst.b	d1
	beq.s	noD1$

	move.b	(a0),d0			;wait (on first SPI_OUT) 

	move.b	#0,(a0)			;if( address & 0x80 ) spi_out(0)
noD1$
	move.b	(a0),d0			;wait (on first or second SPI_OUT)

	move.b	#_OPT_READ_CONST,(a0)	;$ff, provide clock
	lea.l	SDCARD_DATA_R,a0	;SPI_OUT
	move.b	(a0),d0

	move.l	(sp)+,d1		;ensure the 10ns CS hold time

	SPI_DISABLE

	move.l	(sp)+,a0
	rts



;------------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------------
;
; Generic SPI HW independent functions
;
;------------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------------

;
;delay at least D0 microseconds (<= $0fffffff or ~268.000.000)
;
; Important:
;  No guarantee can be given right now by how much the given delay time is exceeded.
;  All this function tries to achieve is to wait long enough in order to meet certain 
;  mimimal hardware delay requirements
;
; Notes: 
; 
;Input:  D0 = delay in microseconds
;Output: void
_enc28j60l_UMinDelay:
	movem.l	d0/d1/a0,-(sp)

	ifne	1
	;
	; this one relies on CIA access time 1.4 us
	;
        moveq   #-1,d1
        lsr.l   #4,d1           ;$0fffffff
        cmp.l   d1,d0
        blo.s   .ok             ;unsigned compare
        move.l  d1,d0           ;268 seconds
.ok
        lsl.l   #3,d0           ;us * 8

        moveq   #12,d1          ;1.5
.loop
        eor.b   #7,d1           ;switch between 12/8 and 11/8 -> 1.4375 effective count per CIA access
        tst.b   $BFE301         ;1 CIA access = 1.4 us
        sub.l   d1,d0           ;us*8 - (1.5 or 1.375)*8
        bgt.s   .loop

	else
	;
	; method used with sdnet 0.7-1.0 -> count dependent on Amiga screen mode
	;
	;  - A nano version should be implemented as well (280 ns horizontal resolution)
	;  - when the Amiga runs NON doublescan modes (OCS or disabled in P96), then this
	;    loop takes twice as long as desired
	;  -> re-implement using ECLOCK of timer.device
		lea	OCS_VHPOSR,a0	;$dff006

newpos$		move.b	(a0),d1
oldpos$		cmp.b	(a0),d1		;compare changes in vertical position
	        beq.s	oldpos$	
		sub.l	#32,d0		;32 microseconds per line (1/31.25 kHz) - in non doublescan this would need to be 64=1/15.625 kHz
		bgt.s	newpos$
	endc

	movem.l	(sp)+,d0/d1/a0
	rts


;static uint8_t _enc28j60l_ReadRegByte (uint8_t address) {
;    _enc28j60l_SetBank(address);
;    return readOp(ENC28J60_READ_CTRL_REG, address);
;}
;In
; D0 = Address
;Out
; D0 = Result
_enc28j60l_ReadRegByte:
	move.l	d3,-(sp)

	bsr	_enc28j60l_SetBank

	move.b	#ENC28J60_READ_CTRL_REG,d3
	bsr	_enc28j60l_ReadOp

	move.l	(sp)+,d3
	rts


;In
; D0 = Address.b
;Out
_enc28j60l_WriteRegByte:
	move.l	d3,-(sp)

	bsr	_enc28j60l_SetBank	;D0 = Address

	moveq	#ENC28J60_WRITE_CTRL_REG,d3
	bsr	_enc28j60l_WriteOp

	move.l	(sp)+,d3
	rts


;In
; D0 = Address
;Out
; D0 = Result
_enc28j60l_ReadReg:
	movem.l	d1/d3/d4,-(sp)

	bsr	_enc28j60l_SetBank

	moveq	#1,d4
	add.b	d0,d4

	move.b	#ENC28J60_READ_CTRL_REG,d3
	bsr	_enc28j60l_ReadOp
	move.b	d0,d1

	move.b	d4,d0
	move.b	#ENC28J60_READ_CTRL_REG,d3
	bsr	_enc28j60l_ReadOp

	lsl.w	#8,d0
	move.b	d1,d0

	movem.l	(sp)+,d1/d3/d4
	rts


;
; D0 = Address.b
; D1 = Data.w
;
_enc28j60l_WriteReg:

	movem.l	d0/d1/d3,-(sp)

	bsr	_enc28j60l_SetBank	;D0 = Address

	moveq	#ENC28J60_WRITE_CTRL_REG,d3
	bsr	_enc28j60l_WriteOp

	lsr	#8,d1
	addq.b	#1,d0
	moveq	#ENC28J60_WRITE_CTRL_REG,d3
	bsr	_enc28j60l_WriteOp

	movem.l	(sp)+,d0/d1/d3
	rts

;
;
; Select proper register bank for subsequent write
; ignores bank switching request for ECON1
;
; Input: D0 = Address
_enc28j60l_SetBank:
	cmp.b	#ECON1,d0	;these are in all 4 banks,
	beq.s	rts$		;frequently used are ECON1,EIR
	cmp.b	#EIR,d0		;
	beq.s	rts$		;

	movem.l	d0/d1/d3-d5,-(sp)

	moveq	#BANK_MASK,d1	;current bank
	and.w	d0,d1		;address & BANK_MASK
	lsr.w	#5,d1		;>>5
;	cmp.b	curbank,d1	;didn't work reliably, disabled for now
;	beq.s	skip$

	move.l	d0,d4	;address
	move.l	d1,d5	;

	move.w	#ENC28J60_BIT_FIELD_CLR,d3
	moveq	#ECON1,d0
	moveq	#ECON1_BSEL1|ECON1_BSEL0,d1
	bsr	_enc28j60l_WriteOp


	move.w	#ENC28J60_BIT_FIELD_SET,d3
	moveq	#ECON1,d0
	moveq	#BANK_MASK,d1	;current bank
	and.w	d4,d1		;address & BANK_MASK
	lsr.w	#5,d1		;>>5
	move.b	d1,curbank	;
	bsr	_enc28j60l_WriteOp

skip$
	movem.l	(sp)+,d0/d1/d3-d5
rts$
	rts



;	section	data,data

curbank:	dc.b	0
		cnop	0,4



	END
