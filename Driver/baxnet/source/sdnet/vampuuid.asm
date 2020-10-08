; ------------------------------------------------------------------------------
; | Flash ROM Read Serial Number                                               |
; | Copyright (c) 2016 APOLLO TEAM                                             |
; | Christoph Höhne, modifications by Henryk Richter                           |
; ------------------------------------------------------------------------------

FLASH_ID                 EQU   $14 ;cmd=0xAB
LARGE_FLASH_ID		 EQU   $16

FLASH_ADR_W              EQU   $00DFF1F8
FLASH_ADR_R              EQU   $00DFF1FA
FLASH_MAGIC              EQU   $42420000
FLASH_CS                 EQU   $1
FLASH_CSn                EQU   $0
FLASH_CLK                EQU   $2
FLASH_CLKn               EQU   $0
FLASH_MOSI               EQU   $4
FLASH_MOSIn              EQU   $0
FLASH_MISO               EQU   $1
FLASH_MISOn              EQU   $0

;VAMPIRE_MAGIC            EQU   ascii "Vamp"
VAMPIRE_MAGIC            EQU   $56616D70

VAMPIDREG	EQU	$DFF3FC
V4_ID1		EQU	$DFF3F4
V4_ID2		EQU	$DFF3F0

BOARDID_V600	EQU	1
BOARDID_V500	EQU	2
BOARDID_V4SA	EQU	5
BOARDID_V1200	EQU	6

	xdef	_vampire_UUID

;-------- Get 64 Bit unique ID --------------------
; Input
;  A1 - pointer to 8 Bytes storage
; Output
;  D0 = 0 - failure
;      >0 - success
_vampire_UUID:
	movem.l	d1-a6,-(sp)	;lazy register store
	lea	VAMPIDREG,a0

	move.w	(a0),d0		;ID reg empty?
	blt.s	v2$		;yes, try flash method
	lsr.w	#8,d0
	cmp.b	#BOARDID_V4SA,d0
	bne.s	v2$
;	cmp.w	#$2FF,d0	;Core type << 8 | Clock_Multiplier
;	ble.s	v2$

	move.l	V4_ID1-VAMPIDREG(a0),d0
	move.l	V4_ID2-VAMPIDREG(a0),d1
	bra.s	common$
v2$
	bsr	flashreadid
	cmpi.b	#FLASH_ID,d0
	beq.s	goodflash$
	cmpi.b	#LARGE_FLASH_ID,d0
	bne.s	error$
goodflash$
	bsr	flashuniqueid
common$
	move.l	d0,(a1)
	move.l	d1,4(a1)

	moveq	#1,d0
	bra.s	return$

error$:
	moveq	#0,d0
return$
	movem.l	(sp)+,d1-a6
	rts

; uint8_t flashreadid(void)
;   D0
flashreadid:
        move.b  #$AB,d1
        bsr     flashbyte
        bsr     flashbyte
        bsr     flashbyte
        bsr     flashbyte
        bsr     flashbyte
        bsr     flashdeselect
        rts

; uint64_t flashuniqueid(void)
;  D0/D1
flashuniqueid:
	move.b	#$5A,d1
	bsr	flashbyte
	clr	d1
	bsr	flashbyte
	clr	d1
	bsr	flashbyte
	move.b	#$F8,d1
	bsr	flashbyte
	bsr	flashbyte
	bsr	flashbyte
	lsl.l	#8,d1
	bsr	flashbyte
	lsl.l	#8,d1
	bsr	flashbyte
	lsl.l	#8,d1
	bsr	flashbyte
	move.l	d1,d3
	bsr	flashbyte
	lsl.l	#8,d1
	bsr	flashbyte
	lsl.l	#8,d1
	bsr	flashbyte
	lsl.l	#8,d1
	bsr	flashbyte
	move.l	d3,d0
	bsr	flashdeselect
	rts

; void flashselect(void)
;flashselect:
;	move.l	#FLASH_MAGIC+FLASH_CSn+FLASH_CLKn,d0
;	move.l	d0,FLASH_ADR_W
;	rts

; void flashdeselect(void)
flashdeselect:
	move.l	#FLASH_MAGIC+FLASH_CS+FLASH_CLK,d3
	move.l	d3,FLASH_ADR_W
	rts

; uint8_t flashbyte(uint8_t)
;   D0                D1
flashbyte:
	moveq	#7,d2
	;bsr	flashselect
	move.l	#FLASH_MAGIC+FLASH_CSn+FLASH_CLKn,d0
	move.l	d0,FLASH_ADR_W
flashbyteloop:
	lsl.b	#1,d1
	scs.b	d0
	andi.w	#FLASH_MOSI,d0
	move.l	d0,FLASH_ADR_W
	ori.w	#FLASH_CLK,d0
	move.l	d0,FLASH_ADR_W
	or.w	FLASH_ADR_R,d1
	dbra	d2,flashbyteloop
	move.b	d1,d0
	rts

