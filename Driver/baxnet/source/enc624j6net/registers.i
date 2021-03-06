; ------------------------------------------------------------------------------
; | Lowlevel Access to memory mapped ENC624J600 in PSP mode                    |
; | Henryk Richter <henryk.richter@gmx.net>                                    |
; ------------------------------------------------------------------------------
; register definition section

; set/clr register offsets
SET_OFFSET	EQU	$0100
CLR_OFFSET	EQU	$0180

; Register definitions

; SPI Bank 0 registers --------
ETXST		EQU    $7E00
ETXSTL		EQU    $7E00
ETXSTH		EQU    $7E01
ETXLEN		EQU    $7E02
ETXLENL		EQU    $7E02
ETXLENH		EQU    $7E03
ERXST		EQU    $7E04
ERXSTL		EQU    $7E04
ERXSTH		EQU    $7E05
ERXTAIL		EQU    $7E06
ERXTAILL	EQU    $7E06
ERXTAILH	EQU    $7E07
ERXHEAD		EQU    $7E08
ERXHEADL	EQU    $7E08
ERXHEADH	EQU    $7E09
EDMAST		EQU    $7E0A
EDMASTL		EQU    $7E0A
EDMASTH		EQU    $7E0B
EDMALEN		EQU    $7E0C
EDMALENL	EQU    $7E0C
EDMALENH	EQU    $7E0D
EDMADST		EQU    $7E0E
EDMADSTL	EQU    $7E0E
EDMADSTH	EQU    $7E0F
EDMACS		EQU    $7E10
EDMACSL		EQU    $7E10
EDMACSH		EQU    $7E11
ETXSTAT		EQU    $7E12
ETXSTATL	EQU    $7E12
ETXSTATH	EQU    $7E13
ETXWIRE		EQU    $7E14
ETXWIREL	EQU    $7E14
ETXWIREH	EQU    $7E15

; SPI all bank registers
EUDAST		EQU    $7E16
EUDASTL		EQU    $7E16
EUDASTH		EQU    $7E17
EUDAND		EQU    $7E18
EUDANDL		EQU    $7E18
EUDANDH		EQU    $7E19
ESTAT		EQU    $7E1A
ESTATL		EQU    $7E1A
ESTATH		EQU    $7E1B
EIR		EQU    $7E1C
EIRL		EQU    $7E1C
EIRH		EQU    $7E1D
ECON1		EQU    $7E1E
ECON1L		EQU    $7E1E
ECON1H		EQU    $7E1F


; SPI Bank 1 registers -----
EHT1		EQU    $7E20
EHT1L		EQU    $7E20
EHT1H		EQU    $7E21
EHT2		EQU    $7E22
EHT2L		EQU    $7E22
EHT2H		EQU    $7E23
EHT3		EQU    $7E24
EHT3L		EQU    $7E24
EHT3H		EQU    $7E25
EHT4		EQU    $7E26
EHT4L		EQU    $7E26
EHT4H		EQU    $7E27
EPMM1		EQU    $7E28
EPMM1L		EQU    $7E28
EPMM1H		EQU    $7E29
EPMM2		EQU    $7E2A
EPMM2L		EQU    $7E2A
EPMM2H		EQU    $7E2B
EPMM3		EQU    $7E2C
EPMM3L		EQU    $7E2C
EPMM3H		EQU    $7E2D
EPMM4		EQU    $7E2E
EPMM4L		EQU    $7E2E
EPMM4H		EQU    $7E2F
EPMCS		EQU    $7E30
EPMCSL		EQU    $7E30
EPMCSH		EQU    $7E31
EPMO		EQU    $7E32
EPMOL		EQU    $7E32
EPMOH		EQU    $7E33
ERXFCON		EQU    $7E34
ERXFCONL	EQU    $7E34
ERXFCONH	EQU    $7E35

; SPI all bank registers from 0x36 to 0x3F
; SPI Bank 2 registers -----
MACON1		EQU    $7E40
MACON1L		EQU    $7E40
MACON1H		EQU    $7E41
MACON2		EQU    $7E42
MACON2L		EQU    $7E42
MACON2H		EQU    $7E43
MABBIPG		EQU    $7E44
MABBIPGL	EQU    $7E44
MABBIPGH	EQU    $7E45
MAIPG		EQU    $7E46
MAIPGL		EQU    $7E46
MAIPGH		EQU    $7E47
MACLCON		EQU    $7E48
MACLCONL	EQU    $7E48
MACLCONH	EQU    $7E49
MAMXFL		EQU    $7E4A
MAMXFLL		EQU    $7E4A
MAMXFLH		EQU    $7E4B
MICMD		EQU    $7E52
MICMDL		EQU    $7E52
MICMDH		EQU    $7E53
MIREGADR	EQU    $7E54
MIREGADRL	EQU    $7E54
MIREGADRH	EQU    $7E55

; SPI all bank registers from 0x56 to 0x5F
; SPI Bank 3 registers -----
MAADR3		EQU    $7E60
MAADR3L		EQU    $7E60
MAADR3H		EQU    $7E61
MAADR2		EQU    $7E62
MAADR2L		EQU    $7E62
MAADR2H		EQU    $7E63
MAADR1		EQU    $7E64
MAADR1L		EQU    $7E64
MAADR1H		EQU    $7E65
MIWR		EQU    $7E66
MIWRL		EQU    $7E66
MIWRH		EQU    $7E67
MIRD		EQU    $7E68
MIRDL		EQU    $7E68
MIRDH		EQU    $7E69
MISTAT		EQU    $7E6A
MISTATL		EQU    $7E6A
MISTATH		EQU    $7E6B
EPAUS		EQU    $7E6C
EPAUSL		EQU    $7E6C
EPAUSH		EQU    $7E6D
ECON2		EQU    $7E6E
ECON2L		EQU    $7E6E
ECON2H		EQU    $7E6F
ERXWM		EQU    $7E70
ERXWML		EQU    $7E70
ERXWMH		EQU    $7E71
EIE		EQU    $7E72
EIEL		EQU    $7E72
EIEH		EQU    $7E73
EIDLED		EQU    $7E74
EIDLEDL		EQU    $7E74
EIDLEDH		EQU    $7E75

; SPI all bank registers from 0x66 to 0x6F

; SPI Non-banked Special Function Registers
EGPDATA		EQU    $7E80
EGPDATAL	EQU    $7E80
ERXDATA		EQU    $7E82
ERXDATAL	EQU    $7E82
EUDADATA	EQU    $7E84
EUDADATAL	EQU    $7E84
EGPRDPT		EQU    $7E86
EGPRDPTL	EQU    $7E86
EGPRDPTH	EQU    $7E87
EGPWRPT		EQU    $7E88
EGPWRPTL	EQU    $7E88
EGPWRPTH	EQU    $7E89
ERXRDPT		EQU    $7E8A
ERXRDPTL	EQU    $7E8A
ERXRDPTH	EQU    $7E8B
ERXWRPT		EQU    $7E8C
ERXWRPTL	EQU    $7E8C
ERXWRPTH	EQU    $7E8D
EUDARDPT	EQU    $7E8E
EUDARDPTL	EQU    $7E8E
EUDARDPTH	EQU    $7E8F
EUDAWRPT	EQU    $7E90
EUDAWRPTL	EQU    $7E90
EUDAWRPTH	EQU    $7E91

;;;;;;;;;;;;;;;;;;;;;;;;;;
; ENC424J600/624J600 register bits
;;;;;;;;;;;;;;;;;;;;;;;;;;
; ESTAT bits ----------
ESTAT_INT		EQU    (1<<15)
ESTAT_FCIDLE	EQU    (1<<14)
ESTAT_RXBUSY	EQU    (1<<13)
ESTAT_CLKRDY	EQU    (1<<12)
ESTAT_RSTDONE	EQU    (1<<11)
ESTAT_PHYDPX	EQU    (1<<10)
ESTAT_PHYRDY	EQU    (1<<9)
ESTAT_PHYLNK	EQU    (1<<8)
ESTAT_PKTCNT7	EQU    (1<<7)
ESTAT_PKTCNT6	EQU    (1<<6)
ESTAT_PKTCNT5	EQU    (1<<5)
ESTAT_PKTCNT4	EQU    (1<<4)
ESTAT_PKTCNT3	EQU    (1<<3)
ESTAT_PKTCNT2	EQU    (1<<2)
ESTAT_PKTCNT1	EQU    (1<<1)
ESTAT_PKTCNT0	EQU    (1)

; EIR bits ------------
EIR_CRYPTEN		EQU    (1<<15)
EIR_MODEXIF		EQU    (1<<14)
EIR_HASHIF		EQU    (1<<13)
EIR_AESIF		EQU    (1<<12)
EIR_LINKIF		EQU    (1<<11)
EIR_PRDYIF		EQU    (1<<10)
EIR_r9			EQU    (1<<9)
EIR_r8			EQU    (1<<8)
EIR_r7			EQU    (1<<7)
EIR_PKTIF		EQU    (1<<6)
EIR_DMAIF		EQU    (1<<5)
EIR_r4			EQU    (1<<4)
EIR_TXIF		EQU    (1<<3)
EIR_TXABTIF		EQU    (1<<2)
EIR_RXABTIF		EQU    (1<<1)
EIR_PCFULIF		EQU    (1)

; ECON1 bits ----------
ECON1_MODEXST	EQU    (1<<15)
ECON1_HASHEN	EQU    (1<<14)
ECON1_HASHOP	EQU    (1<<13)
ECON1_HASHLST	EQU    (1<<12)
ECON1_AESST		EQU    (1<<11)
ECON1_AESOP1	EQU    (1<<10)
ECON1_AESOP0	EQU    (1<<9)
ECON1_PKTDEC	EQU    (1<<8)
ECON1_FCOP1		EQU    (1<<7)
ECON1_FCOP0		EQU    (1<<6)
ECON1_DMAST		EQU    (1<<5)
ECON1_DMACPY	EQU    (1<<4)
ECON1_DMACSSD	EQU    (1<<3)
ECON1_DMANOCS	EQU    (1<<2)
ECON1_TXRTS		EQU    (1<<1)
ECON1_RXEN		EQU    (1)

; ETXSTAT bits --------
ETXSTAT_r12		EQU    (1<<12)
ETXSTAT_r11		EQU    (1<<11)
ETXSTAT_LATECOL	EQU    (1<<10)
ETXSTAT_MAXCOL	EQU    (1<<9)
ETXSTAT_EXDEFER	EQU    (1<<8)
ETXSTAT_DEFER	EQU    (1<<7)
ETXSTAT_r6		EQU    (1<<6)
ETXSTAT_r5		EQU    (1<<5)
ETXSTAT_CRCBAD	EQU    (1<<4)
ETXSTAT_COLCNT3 EQU    (1<<3)
ETXSTAT_COLCNT2 EQU    (1<<2)
ETXSTAT_COLCNT1 EQU    (1<<1)
ETXSTAT_COLCNT0 EQU    (1)

; ERXFCON bits --------
ERXFCON_HTEN	EQU    (1<<15)
ERXFCON_MPEN	EQU    (1<<14)
ERXFCON_NOTPM	EQU    (1<<12)
ERXFCON_PMEN3	EQU    (1<<11)
ERXFCON_PMEN2	EQU    (1<<10)
ERXFCON_PMEN1	EQU    (1<<9)
ERXFCON_PMEN0	EQU    (1<<8)
ERXFCON_CRCEEN	EQU    (1<<7)
ERXFCON_CRCEN	EQU    (1<<6)
ERXFCON_RUNTEEN	EQU    (1<<5)
ERXFCON_RUNTEN	EQU    (1<<4)
ERXFCON_UCEN	EQU    (1<<3)
ERXFCON_NOTMEEN	EQU    (1<<2)
ERXFCON_MCEN	EQU    (1<<1)
ERXFCON_BCEN	EQU    (1)

; MACON1 bits ---------
MACON1_r15		EQU    (1<<15)
MACON1_r14		EQU    (1<<14)
MACON1_r11		EQU    (1<<11)
MACON1_r10		EQU    (1<<10)
MACON1_r9		EQU    (1<<9)
MACON1_r8		EQU    (1<<8)
MACON1_LOOPBK	EQU    (1<<4)
MACON1_r3		EQU    (1<<3)
MACON1_RXPAUS	EQU    (1<<2)
MACON1_PASSALL	EQU    (1<<1)
MACON1_r0		EQU    (1)

; MACON2 bits ---------
MACON2_DEFER	EQU    (1<<14)
MACON2_BPEN		EQU    (1<<13)
MACON2_NOBKOFF	EQU    (1<<12)
MACON2_r9		EQU    (1<<9)
MACON2_r8		EQU    (1<<8)
MACON2_PADCFG2	EQU    (1<<7)
MACON2_PADCFG1	EQU    (1<<6)
MACON2_PADCFG0	EQU    (1<<5)
MACON2_TXCRCEN	EQU    (1<<4)
MACON2_PHDREN	EQU    (1<<3)
MACON2_HFRMEN	EQU    (1<<2)
MACON2_r1		EQU    (1<<1)
MACON2_FULDPX	EQU    (1)

; MABBIPG bits --------
MABBIPG_BBIPG6  EQU    (1<<6)
MABBIPG_BBIPG5  EQU    (1<<5)
MABBIPG_BBIPG4  EQU    (1<<4)
MABBIPG_BBIPG3  EQU    (1<<3)
MABBIPG_BBIPG2  EQU    (1<<2)
MABBIPG_BBIPG1  EQU    (1<<1)
MABBIPG_BBIPG0  EQU    (1)

; MAIPG bits ----------
MAIPG_r14		EQU    (1<<14)
MAIPG_r13		EQU    (1<<13)
MAIPG_r12		EQU    (1<<12)
MAIPG_r11		EQU    (1<<11)
MAIPG_r10		EQU    (1<<10)
MAIPG_r9		EQU    (1<<9)
MAIPG_r8		EQU    (1<<8)
MAIPG_IPG6		EQU    (1<<6)
MAIPG_IPG5		EQU    (1<<5)
MAIPG_IPG4		EQU    (1<<4)
MAIPG_IPG3		EQU    (1<<3)
MAIPG_IPG2		EQU    (1<<2)
MAIPG_IPG1		EQU    (1<<1)
MAIPG_IPG0		EQU    (1)

; MACLCON bits --------
MACLCON_r13		EQU    (1<<13)
MACLCON_r12		EQU    (1<<12)
MACLCON_r11		EQU    (1<<11)
MACLCON_r10		EQU    (1<<10)
MACLCON_r9		EQU    (1<<9)
MACLCON_r8		EQU    (1<<8)
MACLCON_MAXRET3	EQU    (1<<3)
MACLCON_MAXRET2	EQU    (1<<2)
MACLCON_MAXRET1	EQU    (1<<1)
MACLCON_MAXRET0	EQU    (1)

; MICMD bits ----------
MICMD_MIISCAN	EQU    (1<<1)
MICMD_MIIRD		EQU    (1)

; MIREGADR bits -------
MIREGADR_r12	EQU    (1<<12)
MIREGADR_r11	EQU    (1<<11)
MIREGADR_r10	EQU    (1<<10)
MIREGADR_r9		EQU    (1<<9)
MIREGADR_r8		EQU    (1<<8)
MIREGADR_PHREG4	EQU    (1<<4)
MIREGADR_PHREG3	EQU    (1<<3)
MIREGADR_PHREG2	EQU    (1<<2)
MIREGADR_PHREG1	EQU    (1<<1)
MIREGADR_PHREG0	EQU    (1)

; MISTAT bits ---------
MISTAT_r3		EQU    (1<<3)
MISTAT_NVALID	EQU    (1<<2)
MISTAT_SCAN		EQU    (1<<1)
MISTAT_BUSY		EQU    (1)

; ECON2 bits ----------
ECON2_ETHEN		EQU    (1<<15)
ECON2_STRCH		EQU    (1<<14)
ECON2_TXMAC		EQU    (1<<13)
ECON2_SHA1MD5	EQU    (1<<12)
ECON2_COCON3	EQU    (1<<11)
ECON2_COCON2	EQU    (1<<10)
ECON2_COCON1	EQU    (1<<9)
ECON2_COCON0	EQU    (1<<8)
ECON2_AUTOFC	EQU    (1<<7)
ECON2_TXRST		EQU    (1<<6)
ECON2_RXRST		EQU    (1<<5)
ECON2_ETHRST	EQU    (1<<4)
ECON2_MODLEN1	EQU    (1<<3)
ECON2_MODLEN0	EQU    (1<<2)
ECON2_AESLEN1	EQU    (1<<1)
ECON2_AESLEN0	EQU    (1)

; ERXWM bits ----------
ERXWM_RXFWM7	EQU    (1<<15)
ERXWM_RXFWM6	EQU    (1<<14)
ERXWM_RXFWM5	EQU    (1<<13)
ERXWM_RXFWM4	EQU    (1<<12)
ERXWM_RXFWM3	EQU    (1<<11)
ERXWM_RXFWM2	EQU    (1<<10)
ERXWM_RXFWM1	EQU    (1<<9)
ERXWM_RXFWM0	EQU    (1<<8)
ERXWM_RXEWM7	EQU    (1<<7)
ERXWM_RXEWM6	EQU    (1<<6)
ERXWM_RXEWM5	EQU    (1<<5)
ERXWM_RXEWM4	EQU    (1<<4)
ERXWM_RXEWM3	EQU    (1<<3)
ERXWM_RXEWM2	EQU    (1<<2)
ERXWM_RXEWM1	EQU    (1<<1)
ERXWM_RXEWM0	EQU    (1)

; EIE bits ------------
EIE_INTIE		EQU    (1<<15)
EIE_MODEXIE		EQU    (1<<14)
EIE_HASHIE		EQU    (1<<13)
EIE_AESIE		EQU    (1<<12)
EIE_LINKIE		EQU    (1<<11)
EIE_PRDYIE		EQU    (1<<10)
EIE_r9			EQU    (1<<9)
EIE_r8			EQU    (1<<8)
EIE_r7			EQU    (1<<7)
EIE_PKTIE		EQU    (1<<6)
EIE_DMAIE		EQU    (1<<5)
EIE_r4			EQU    (1<<4)
EIE_TXIE		EQU    (1<<3)
EIE_TXABTIE		EQU    (1<<2)
EIE_RXABTIE		EQU    (1<<1)
EIE_PCFULIE		EQU    (1)

; EIDLED bits ---------
EIDLED_LACFG3	EQU    (1<<15)
EIDLED_LACFG2	EQU    (1<<14)
EIDLED_LACFG1	EQU    (1<<13)
EIDLED_LACFG0	EQU    (1<<12)
EIDLED_LBCFG3	EQU    (1<<11)
EIDLED_LBCFG2	EQU    (1<<10)
EIDLED_LBCFG1	EQU    (1<<9)
EIDLED_LBCFG0	EQU    (1<<8)
EIDLED_DEVID2	EQU    (1<<7)
EIDLED_DEVID1	EQU    (1<<6)
EIDLED_DEVID0	EQU    (1<<5)
EIDLED_REVID4	EQU    (1<<4)
EIDLED_REVID3	EQU    (1<<3)
EIDLED_REVID2	EQU    (1<<2)
EIDLED_REVID1	EQU    (1<<1)
EIDLED_REVID0	EQU    (1)

; PHY REGISTERS
PHCON1	EQU	0
PHSTAT1	EQU	1
PHANA	EQU	4
PHANLPA	EQU	5
PHANE	EQU	6
PHCON2	EQU	$11
PHSTAT2	EQU	$1b
PHSTAT3	EQU	$1f

; PHCON1 bits ---------
PHCON1_PRST		EQU    (1<<15)
PHCON1_PLOOPBK	EQU    (1<<14)
PHCON1_SPD100	EQU    (1<<13)
PHCON1_ANEN		EQU    (1<<12)
PHCON1_PSLEEP	EQU    (1<<11)
PHCON1_r10		EQU    (1<<10)
PHCON1_RENEG	EQU    (1<<9)
PHCON1_PFULDPX	EQU    (1<<8)
PHCON1_r7		EQU    (1<<7)
PHCON1_r6		EQU    (1<<6)
PHCON1_r5		EQU    (1<<5)
PHCON1_r4		EQU    (1<<4)
PHCON1_r3		EQU    (1<<3)
PHCON1_r2		EQU    (1<<2)
PHCON1_r1		EQU    (1<<1)
PHCON1_r0		EQU    (1)

; PHSTAT1 bits --------
PHSTAT1_r15		EQU    (1<<15)
PHSTAT1_FULL100	EQU    (1<<14)
PHSTAT1_HALF100	EQU    (1<<13)
PHSTAT1_FULL10	EQU    (1<<12)
PHSTAT1_HALF10	EQU    (1<<11)
PHSTAT1_r10		EQU    (1<<10)
PHSTAT1_r9		EQU    (1<<9)
PHSTAT1_r8		EQU    (1<<8)
PHSTAT1_r7		EQU    (1<<7)
PHSTAT1_r6		EQU    (1<<6)
PHSTAT1_ANDONE	EQU    (1<<5)
PHSTAT1_LRFAULT	EQU    (1<<4)
PHSTAT1_ANABLE	EQU    (1<<3)
PHSTAT1_LLSTAT	EQU    (1<<2)
PHSTAT1_r1		EQU    (1<<1)
PHSTAT1_EXTREGS	EQU    (1)

; PHANA bits ----------
PHANA_ADNP		EQU    (1<<15)
PHANA_r14		EQU    (1<<14)
PHANA_ADFAULT	EQU    (1<<13)
PHANA_r12		EQU    (1<<12)
PHANA_ADPAUS1	EQU    (1<<11)
PHANA_ADPAUS0	EQU    (1<<10)
PHANA_r9		EQU    (1<<9)
PHANA_AD100FD	EQU    (1<<8)
PHANA_AD100		EQU    (1<<7)
PHANA_AD10FD	EQU    (1<<6)
PHANA_AD10		EQU    (1<<5)
PHANA_ADIEEE4	EQU    (1<<4)
PHANA_ADIEEE3	EQU    (1<<3)
PHANA_ADIEEE2	EQU    (1<<2)
PHANA_ADIEEE1	EQU    (1<<1)
PHANA_ADIEEE0	EQU    (1)

; PHANLPA bits --------
PHANLPA_LPNP	EQU    (1<<15)
PHANLPA_LPACK	EQU    (1<<14)
PHANLPA_LPFAULT	EQU    (1<<13)
PHANLPA_r12		EQU    (1<<12)
PHANLPA_LPPAUS1	EQU    (1<<11)
PHANLPA_LPPAUS0	EQU    (1<<10)
PHANLPA_LP100T4	EQU    (1<<9)
PHANLPA_LP100FD	EQU    (1<<8)
PHANLPA_LP100	EQU    (1<<7)
PHANLPA_LP10FD	EQU    (1<<6)
PHANLPA_LP10	EQU    (1<<5)
PHANLPA_LPIEEE4	EQU    (1<<4)
PHANLPA_LPIEEE3	EQU    (1<<3)
PHANLPA_LPIEEE2	EQU    (1<<2)
PHANLPA_LPIEEE1	EQU    (1<<1)
PHANLPA_LPIEEE0	EQU    (1)

; PHANE bits ----------
PHANE_r15		EQU    (1<<15)
PHANE_r14		EQU    (1<<14)
PHANE_r13		EQU    (1<<13)
PHANE_r12		EQU    (1<<12)
PHANE_r11		EQU    (1<<11)
PHANE_r10		EQU    (1<<10)
PHANE_r9		EQU    (1<<9)
PHANE_r8		EQU    (1<<8)
PHANE_r7		EQU    (1<<7)
PHANE_r6		EQU    (1<<6)
PHANE_r5		EQU    (1<<5)
PHANE_PDFLT		EQU    (1<<4)
PHANE_r3		EQU    (1<<3)
PHANE_r2		EQU    (1<<2)
PHANE_LPARCD	EQU    (1<<1)
PHANA_LPANABL	EQU    (1)

; PHCON2 bits ---------
PHCON2_r15		EQU    (1<<15)
PHCON2_r14		EQU    (1<<14)
PHCON2_EDPWRDN	EQU    (1<<13)
PHCON2_r12		EQU    (1<<12)
PHCON2_EDTHRES	EQU    (1<<11)
PHCON2_r10		EQU    (1<<10)
PHCON2_r9		EQU    (1<<9)
PHCON2_r8		EQU    (1<<8)
PHCON2_r7		EQU    (1<<7)
PHCON2_r6		EQU    (1<<6)
PHCON2_r5		EQU    (1<<5)
PHCON2_r4		EQU    (1<<4)
PHCON2_r3		EQU    (1<<3)
PHCON2_FRCLNK	EQU    (1<<2)
PHCON2_EDSTAT	EQU    (1<<1)
PHCON2_r0		EQU    (1)

; PHSTAT2 bits ---------
PHSTAT2_r15		EQU    (1<<15)
PHSTAT2_r14		EQU    (1<<14)
PHSTAT2_r13		EQU    (1<<13)
PHSTAT2_r12		EQU    (1<<12)
PHSTAT2_r11		EQU    (1<<11)
PHSTAT2_r10		EQU    (1<<10)
PHSTAT2_r9		EQU    (1<<9)
PHSTAT2_r8		EQU    (1<<8)
PHSTAT2_r7		EQU    (1<<7)
PHSTAT2_r6		EQU    (1<<6)
PHSTAT2_r5		EQU    (1<<5)
PHSTAT2_PLRITY	EQU    (1<<4)
PHSTAT2_r3		EQU    (1<<3)
PHSTAT2_r2		EQU    (1<<2)
PHSTAT2_r1		EQU    (1<<1)
PHSTAT2_r0		EQU    (1)

; PHSTAT3 bits --------
PHSTAT3_r15		EQU    (1<<15)
PHSTAT3_r14		EQU    (1<<14)
PHSTAT3_r13		EQU    (1<<13)
PHSTAT3_r12		EQU    (1<<12)
PHSTAT3_r11		EQU    (1<<11)
PHSTAT3_r10		EQU    (1<<10)
PHSTAT3_r9		EQU    (1<<9)
PHSTAT3_r8		EQU    (1<<8)
PHSTAT3_r7		EQU    (1<<7)
PHSTAT3_r6		EQU    (1<<6)
PHSTAT3_r5		EQU    (1<<5)
PHSTAT3_SPDDPX2	EQU    (1<<4)
PHSTAT3_SPDDPX1	EQU    (1<<3)
PHSTAT3_SPDDPX0	EQU    (1<<2)
PHSTAT3_r1		EQU    (1<<1)
PHSTAT3_r0		EQU    (1)

;------------------------------ internal defaults ----------------------------------
;33.333Mhz clock out frequency
CLOCK_33_CLR	EQU	(ECON2_COCON3|ECON2_COCON2|ECON2_COCON1)	;clr mask
CLOCK_33_SET	EQU	(ECON2_COCON0)					;set mask
;25 MHz
CLOCK_25_CLR	EQU	(ECON2_COCON3|ECON2_COCON2|ECON2_COCON0)	;clr mask
CLOCK_25_SET	EQU	(ECON2_COCON1)					;set mask
;20 MHz
CLOCK_20_CLR	EQU	(ECON2_COCON3|ECON2_COCON2)			;clr mask
CLOCK_20_SET	EQU	(ECON2_COCON1|ECON2_COCON0)			;set mask
;16 MHz
CLOCK_16_CLR	EQU	(ECON2_COCON3|ECON2_COCON0|ECON2_COCON1)	;clr mask
CLOCK_16_SET	EQU	(ECON2_COCON2)					;set mask
;4 MHz (after reset)
CLOCK_4_CLR	EQU	(ECON2_COCON2)					;clr mask
CLOCK_4_SET	EQU	(ECON2_COCON3|ECON2_COCON0|ECON2_COCON1)	;set mask

; changed to 25 MHz for ZIII firmware
CLOCK_DEF_CLR	EQU	CLOCK_25_CLR
CLOCK_DEF_SET	EQU	CLOCK_25_SET
