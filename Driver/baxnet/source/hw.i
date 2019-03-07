;
;  hw.i
;
;  (C) 2018 Henryk Richter <henryk.richter@gmx.net>
;
;  Hardware dependent definitions
;
;
;
	ifnd	_INC_HW_I
_INC_HW_I	EQU	1

;/* states for each NIC speed */
HW_MAC_OFF       EQU 0 ; this speed is inactive
HW_MAC_ON        EQU 1 ; on, half duplex
HW_MAC_ON_FDX    EQU 2 ; on, full duplex
HW_MAC_ON_FDX_FC EQU 3 ; on, full duplex, flow-control

;
;
;  MAC definitions 
;  (flag Mask)
;  -> conceptually, this layout can be used for current
;     running mode as well as autonegotiation capabilities 
;     masks for local NIC and peer
;
HW_MAC_INVALID         EQU -1 ; no info available
HW_MACB_LINK_UP        EQU 0
HW_MACB_AUTONEGOTIATION EQU 1
HW_MAC_LINK_UP         EQU  (1<<HW_MACB_LINK_UP)        ; link is up
HW_MAC_AUTONEGOTIATION  EQU  (1<<HW_MACB_AUTONEGOTIATION) ; autonegotiation enabled   

; two bits each to handle the four different states per speed 
HW_MACB_SPEED10      EQU 2  ; bit for 10 MBit/s mask    
HW_MACB_SPEED100     EQU 4  ; bit for 100 MBit/s mask   
HW_MACB_SPEED1000    EQU 6  ; ... 
HW_MACB_SPEED2500    EQU 8
HW_MACB_SPEED5000    EQU 10
HW_MACB_SPEED10000   EQU 12 ; Am I optimistic ? 
HW_MACB_SPEEDS_END   EQU 14 ; this is the next free bit 
HW_MAC_SPEEDSMASK    EQU ((1<<HW_MACB_SPEEDS_END)-4)
HW_MAGIC_WORD 			 EQU $B00BDEED
	endc	;_INC_HW_I
