/*
 Microchip ENC28J60 Ethernet Interface Driver
 Author: Henryk Richter
 Copyright: GPL V2

 Notes: when I ran into timing issues in early development, I moved over to quickly
        testable ASM code for SPI communication. I didn't bother porting that stuff
	back into C code. So this file now references enc28j60l.asm for lowlevel direct 
	hardware access.
	As a consequence, the only functions needed from spi.c are the spi_init() and
	spi_disable_eth().

 TODO:  - support for power down when unused, i.e. offline
        - replace busy waiting loop after reset

 Based on work by
  Christian Vogelgsaang
  Guido Socher
  Pascal Stang (see enc28j60.c in AVRlib library for the original file)
*/
#undef _DO_DUMP

/* used for Delay() */
#include <dos/dos.h>
#include <proto/dos.h>

#ifdef _DO_DUMP
#include <stdio.h>
#endif

#include "compiler.h"
#include "enc28j60.h"
#include "enc28j60l.h" /* import lowlevel asm functions */
/* #include "spi.h" */ /* no direct use anymore, just for init purposes) */
#include "device.h"

/*
  The RXSTART_INIT must be zero. See Rev. B4 Silicon Errata point 5.
  Buffer boundaries applied to internal 8K ram
  the entire available packet buffer space is allocated

  rx packet layout
  6 byte heaader
  1518 
  sum: 1524
*/

#define RXSTART_INIT        0x0000  /*  start of RX buffer, room for 4 packets */
#define RXSTOP_INIT         0x19FF  /*  end of RX buffer */
                            
#define TXSTART_INIT        0x1A00  /*  start of TX buffer, room for 1 packet */
#define TXSTOP_INIT         0x1FFF  /*  end of TX buffer */
                            
/*  max frame length which the conroller will accept: */
/*  (note: maximum ethernet frame length would be 1518 when VLAN tagging is unused) */
#define MAX_FRAMELEN      1518 

static u16 gNextPacketPtr;
static u08 is_full_duplex;
static u08 rev;

struct enc28j60_initvars {
	u16	have_recv;
	u08	last_mac[6];
	u08	macon1;
	u08	macon2;
	u08	macon3;
	u08	macon4;
};
static struct enc28j60_initvars initvars;


static void enc28j60_recv_reset( struct enc28j60_initvars *iv, u08 restart );
static int  enc28j60_check_config( struct enc28j60_initvars *iv );


static uint8_t readOp (uint8_t op, uint8_t address) 
{
 return enc28j60l_ReadOp( op, address );
}

static void writeOp (uint8_t op, uint8_t address, uint8_t data)
{
    enc28j60l_WriteOp( op, address,data );
}

static void readBuf(uint16_t len, uint8_t* data)
{
    enc28j60l_ReadBuffer( data, (unsigned int)len ); 
}

void SetBank (uint8_t address)
{
 enc28j60l_SetBank(address);
}

static uint8_t readRegByte (uint8_t address)
{
 return enc28j60l_ReadRegByte( address );
}

#if 0
static uint16_t readReg(uint8_t address) {
	return readRegByte(address) + (readRegByte(address+1) << 8);
}
#endif

static void writeRegByte (uint8_t address, uint8_t data)
{
 enc28j60l_WriteRegByte( address, data );
}

static void writeReg(uint8_t address, uint16_t data)
{
 enc28j60l_WriteReg( address, data );
}

/*
  the PHY registers require special care
*/
static uint16_t readPhyByte (uint8_t address) {
    writeRegByte(MIREGADR, address);
    writeRegByte(MICMD, MICMD_MIIRD);
    while (readRegByte(MISTAT) & MISTAT_BUSY)
        ;
    writeRegByte(MICMD, 0x00);
    return readRegByte(MIRD+1);
}

static void writePhy (uint8_t address, uint16_t data) {
    writeRegByte(MIREGADR, address);
    writeReg(MIWR, data);
    while (readRegByte(MISTAT) & MISTAT_BUSY)
        ;
}

/* just a pass-through to the lowlevel routines */
void enc28j60_SetSPISpeed( unsigned long clock_divider )
{
	enc28j60l_SetSPISpeed( clock_divider );
}

/*  ---------- init ---------- */

/*  Functions to enable/disable broadcast filter bits */
/*  With the bit set, broadcast packets are filtered. */
void enc28j60_broadcast_multicast_filter( u08 flags  )
{
	u08 val = ERXFCON_UCEN|ERXFCON_CRCEN; /* default: crc check and unicast */

	if( flags & PIO_INIT_PROMISC )
	{
		val = 0;
	}
	else
	{
		if( flags & PIO_INIT_BROAD_CAST )
			val |= ERXFCON_BCEN;
		if( flags & PIO_INIT_MULTI_CAST )
			val |= ERXFCON_MCEN;
	}
	writeRegByte(ERXFCON, val );
}

static void enc28j60_recv_reset( struct enc28j60_initvars *iv, u08 restart )
{
	writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_RXEN);
	writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXRST);
	writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_RXRST);
	writeOp(ENC28J60_BIT_FIELD_CLR, ESTAT, ESTAT_TXABRT);

	gNextPacketPtr = RXSTART_INIT;
	writeReg(ERXST,   RXSTART_INIT);
	writeReg(ERXRDPT, RXSTOP_INIT );
	writeReg(ERXND,   RXSTOP_INIT);
	writeOp(ENC28J60_BIT_FIELD_CLR, EIR, EIR_TXERIF|EIR_RXERIF );
	writeReg(MAMXFL, MAX_FRAMELEN);

	writeRegByte(MACON4, iv->macon4 ); /* DEFER bit */
	writeRegByte(MACON3, iv->macon3 );
	writeRegByte(MACON2, iv->macon2 );
	writeRegByte(MACON1, iv->macon1 );

	enc28j60_setMAC( iv->last_mac );

	writePhy(PHIE, PHIE_PGEIE|PHIE_LNKIE );
 	writePhy(PHLCON, 0x3c12 ); /* 3(fixed) C(LinkStat+RX Act) 1(TX Act) 2(Stretch enable) */
 
	/*  prepare flow control */
	writeReg(EPAUS, 20 * 100); /*  100ms */

	if( restart )
		writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

	iv->have_recv = 0;
}


static int  enc28j60_check_config( struct enc28j60_initvars *iv )
{
	int	ret = 1;	/* def: be optimistic */

	/* check RX registers */
	if( !(readOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_RXEN ) )
		ret = 0;
	if( (readRegByte(MACON1)&iv->macon1) != iv->macon1 )
		ret = 0;
	if( (readRegByte(MACON2)&iv->macon2) != iv->macon2 )
		ret = 0;
	if( (readRegByte(MACON3)&iv->macon3) != iv->macon3 )
		ret = 0;

	/* check TX problems */
	if( readRegByte(ESTAT) & ESTAT_TXABRT )
		ret = 0;
	if( readRegByte(EIR) & EIR_TXERIF )
		ret = 0;

	/* Verify MAC */
	if( readRegByte(MAADR5) != iv->last_mac[0] )
		ret = 0;
	if( readRegByte(MAADR4) != iv->last_mac[1] )
		ret = 0;
	if( readRegByte(MAADR3) != iv->last_mac[2] )
		ret = 0;
	if( readRegByte(MAADR2) != iv->last_mac[3] )
		ret = 0;
	if( readRegByte(MAADR1) != iv->last_mac[4] )
		ret = 0;
	if( readRegByte(MAADR0) != iv->last_mac[5] )
		ret = 0;

	return	ret;
}




u08 enc28j60_init(const u08 macaddr[6], u08 flags)
{
  unsigned int count;
  u08 mac3val;
  struct enc28j60_initvars *iv = &initvars;

  enc28j60l_Init(); /*  spi_init(); spi_disable_eth(); */

  {
	int i;
	for(i=0 ; i < 6 ; i++ )
		iv->last_mac[i] = macaddr[i];
  }

  is_full_duplex = (flags & PIO_INIT_FULL_DUPLEX) == PIO_INIT_FULL_DUPLEX;

  /*  soft reset cpu */
  writeOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);

#if 1
  enc28j60l_UMinDelay( 2000 ); /* wait 2 ms */
#else
  /*  Delay(1);*/	/*   _delay_ms(2); // errata B7/2 */
  /* hard-coded delay right now (TODO: import timer functions) */
  for( count = 0 ; count < 20000 ; count++ )
  {
	mac3val |= (u08)*((volatile unsigned short*)(0x00DE0004));
  }
#endif
  
  /*  wait or error */
  count = 0;
  while (!readOp(ENC28J60_READ_CTRL_REG, ESTAT) & ESTAT_CLKRDY) {
    count ++;
    if(count == 0xfff) {
      return PIO_NOT_FOUND;
    }
  }

  writeOp(ENC28J60_WRITE_CTRL_REG, ECON1, 0);
 
  /*  return rev */
  rev = readRegByte(EREVID);
  /*  microchip forgot to step the number on the silcon when they */
  /*  released the revision B7. 6 is now rev B7. We still have */
  /*  to see what they do when they release B8. At the moment */
  /*  there is no B8 out yet */
  if ( (rev < 1) || (rev > 10) )	/* I don't expect a rev > 10 (+1), so I use it to check for module presence */
  	return PIO_NOT_FOUND;	/* important: check this before any writePhy -> no module gets us 0xff back */

  if (rev > 5) ++rev;
 
  writeReg(ETXST, TXSTART_INIT);
  writeReg(ETXND, TXSTOP_INIT);
  
  /*  set packet filter */
  enc28j60_broadcast_multicast_filter( flags );/* flags & (PIO_INIT_BROAD_CAST|PIO_INIT_MULTI_CAST) ); */

  /*  BIST pattern generator? */
  writeReg(EPMM0, 0x303f);
  writeReg(EPMCS, 0xf7f9);

  if(is_full_duplex) {
    writeRegByte(MABBIPG, 0x15);    
    writeReg(MAIPG, 0x0012);
  } else {
    writeRegByte(MABBIPG, 0x12);
    writeReg(MAIPG, 0x0C12);
  }

  /*  MAC init (with flow control) */
  mac3val = MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN;
  if(is_full_duplex) {
    mac3val |= MACON3_FULDPX;
    iv->macon4=0;
  }
  else
  {
    iv->macon4 = 1<<6;	/* DEFER bit */
  }
  iv->macon3 = mac3val;
  iv->macon2 = 0x00;
  iv->macon1 = MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS;

  /*  set packet pointers */
  enc28j60_recv_reset( iv, 0 );


  /*  PHY init */
  if(is_full_duplex) {
    writePhy(PHCON1, PHCON1_PDPXMD);
    writePhy(PHCON2, 0);
  } else {
    writePhy(PHCON1, 0);
    writePhy(PHCON2, PHCON2_HDLDIS);
  }

  writePhy(PHLCON, 0x3c12 ); /* 3(fixed) C(LinkStat+RX Act) 1(TX Act) 2(Stretch enable) */
 
  SetBank(EIR);
  writeOp(ENC28J60_BIT_FIELD_CLR, EIR, EIR_DMAIF|EIR_LINKIF|EIR_TXIF|EIR_TXERIF|EIR_RXERIF|EIR_PKTIF);
  writeOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE); /* |EIE_LINKIE|EIE_TXIE|EIE_TXERIE|EIE_RXERIE); */

  writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

  /* wait a while for "LINK UP" -> break after 200 ms */
  {
   unsigned char status;
   int runs = 100;

   while( runs-- )
   {
    enc28j60_status( PIO_STATUS_LINK_UP, &status );
    if( status )
    	break;

    enc28j60l_UMinDelay( 2000 ); /* wait 2+ ms */
   }
  }


  return PIO_OK;
}

/*  ---------- exit ---------- */

void enc28j60_exit(void)
{
	enc28j60_setOffline();
	enc28j60l_Shutdown();
}

u08 enc28j60_setOnline( void )
{
	SetBank(EIR);
	writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

	return	1;
}

u08 enc28j60_setOffline( void )
{
	SetBank(EIR);
	writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_RXEN);    

	return	1;
}

void enc28j60_setMAC( const u08 macaddr[6] )
{
/*  set mac     */
  writeRegByte(MAADR5, macaddr[0]);
  writeRegByte(MAADR4, macaddr[1]);
  writeRegByte(MAADR3, macaddr[2]);
  writeRegByte(MAADR2, macaddr[3]);
  writeRegByte(MAADR1, macaddr[4]);
  writeRegByte(MAADR0, macaddr[5]);
  SetBank(EIR);
}

void enc28j60_getMAC( u08 macaddr[6] ) /* mainly for debug */
{
/*  set mac     */
 macaddr[0] = readRegByte(MAADR5);
 macaddr[1] = readRegByte(MAADR4);
 macaddr[2] = readRegByte(MAADR3);
 macaddr[3] = readRegByte(MAADR2);
 macaddr[4] = readRegByte(MAADR1);
 macaddr[5] = readRegByte(MAADR0);
}


/*  ---------- control ---------- */

u08 enc28j60_control(u08 control_id, u08 value)
{ 
  switch(control_id) {
    case PIO_CONTROL_FLOW:
      {
        u08 flag;
        if(is_full_duplex) {
          flag = value ? 2 : 3;
        } else {
          flag = value ? 1 : 0;
        }
        writeRegByte(EFLOCON, flag);
        return PIO_OK;
      }
    default:
      return PIO_NOT_FOUND;
  }
}


/*  ---------- status ---------- */

u08 enc28j60_status(u08 status_id, u08 *value)
{
  switch(status_id) {
    case PIO_STATUS_VERSION:
      *value = rev;
      return PIO_OK;
    case PIO_STATUS_LINK_UP:
      *value = (readPhyByte(PHSTAT2) >> 2) & 1;
      return PIO_OK;
    case PIO_STATUS_FULLDPX:
      *value = (readRegByte(MACON3) & MACON3_FULDPX) ? 1 : 0;/*  + (((u16)readRegByte(MACON1+3))<<8); */
      return PIO_OK;
    default:
      *value = 0;
      return PIO_NOT_FOUND;
  }
}

/*  ---------- send ---------- */

u08 enc28j60_send(const u08 *data, u16 size)
{
  /*  wait for tx ready */
  /* We can't do it here like on ATMEL MCUs, simply because
     the Vampire SPI is way faster than 10 MBit/s so that
     we would overwrite a frame still being sent out
     -> wait first, then fill buffer
  */
  while (readOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_TXRTS)
  {
      /* errata sheet issue 12 -> TXRTS never becomes 0 */
      /* ignores errata issue 13 -> frame loss due to late collision */
      /* eworks around errata sheet issue 15 -> ESTAT.LATECOL not set */
      if (readRegByte(EIR) & EIR_TXERIF) {
          writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
          writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
      }
  }

  /* write buffer */
#if 0
  if(0)
  { u08* d = (u08*)data;
    d[size]=0;
    size=(size+1)&0xfffe;  
  }
#endif
  enc28j60l_WriteBuffer( (u08*)data, size, TXSTART_INIT );

  /*  initiate send */
  writeReg(ETXND, TXSTART_INIT+size);
  writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
  return PIO_OK;
}

/*  ---------- recv ---------- */

/* 
   unrecoverable error in receiving, bad pointer or other causes
   - reset the complete receive logic in order to start receiving anew

   input: restart - 0 = don't re-enable RX automatically
                    1 = re-enable RX after resetting
*/

INLINE void next_pkt(void)
{
  
  if (gNextPacketPtr - 1 > RXSTOP_INIT)
  {
#if 1
	enc28j60_recv_reset( &initvars, 1 );
#else
      writeReg(ERXRDPT, RXSTOP_INIT);
      gNextPacketPtr = 0;
#endif
  }  
  else
      writeReg(ERXRDPT, gNextPacketPtr - 1);

  writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);  
}

/* endian agnostic version of header read */
static u08 read_hdr(u16 *got_size)
{
  struct {
      uint8_t nextPacketL;
      uint8_t nextPacketH;
      uint8_t byteCountL;
      uint8_t byteCountH;
      uint8_t statusL;
      uint8_t statusH;
  } header;
  
  readBuf(sizeof header, (uint8_t*) &header);

  gNextPacketPtr  = (u16)header.nextPacketL + (((u16)header.nextPacketH)<<8);
  *got_size = (u16)header.byteCountL + (((u16)header.byteCountH)<<8); /* - 4;*//* -4: remove the CRC count */

  return header.statusL;
}


u08 enc28j60_recv(u08 *data, u16 max_size, u16 *got_size)
{
  u08 status,result;
  u16 len;

  /* EIR->PKTIF would be faster but errata sheet says "unreliable" */
  u08 pcnt = 1+readRegByte(EPKTCNT); /* wraps to 0 for 0xff = unconnected module */
  if( pcnt < 2 ) /* 0xff+1=0x00, 0x00+1=0x01,  0x01+1=0x02 */
  	return PIO_NOT_FOUND;

  writeReg(ERDPT, gNextPacketPtr);

  /*  read chip's packet header */
  status = read_hdr(got_size);

  /*  was a receive error? */
  if ((status & 0x80)==0) 
  {
    /* TODO: recv reset ? */
    next_pkt();
    return PIO_IO_ERR;
  }

  /*  check size */
  len = *got_size;
  result = PIO_OK;
  if(len > max_size) {
    len = max_size;
    result = PIO_TOO_LARGE;
  }

  /*  read packet */
  readBuf(len, data);
#if 0
  {
  	data[len]=0;
  	data[len+1]=0;
  	data[len+2]=0;
  	data[len+3]=0;
  }
#endif

  next_pkt();
  return result;
}

/*  ---------- has_recv ---------- */

u08 enc28j60_has_recv(void)
{
  u08 ret = readRegByte(EPKTCNT);

  if( !ret )
  {
 	initvars.have_recv++;
	if( initvars.have_recv > 10 )
	{	
		/* check for bad config vars and reset hw if that is the case */
		if( !enc28j60_check_config( &initvars ) )
		{
			enc28j60_recv_reset( &initvars, 1 );
		}
	}
  }
  else	
  	initvars.have_recv = 0;

  return ret;
}

void	enc28j60_dump_regs(void)
{
/* this will work only with DOSBase known globally, not when it's redirected to Plipbase */
#ifdef _DO_DUMP
	  u08 val,eir,eie,econ1,econ2,estat;
	  u16 erxrdpt,erxwrpt,erxst,erxnd,mamxfll;
	  u16 macon1,macon3,macon4;
 
	  eir   = readRegByte(EIR);
	  eie   = readRegByte(EIE);
	  econ1 = readRegByte(ECON1);
	  econ2 = readRegByte(ECON2);
	  estat = readRegByte(ESTAT);
	  printf("EIR EIE ECON1 ECON2 ESTAT\n");
	  printf("%02x  %02x  %02x    %02x    %02x\n",
	        eir,eie,econ1,econ2,estat);

	  erxrdpt = readRegByte(ERXRDPT) + (((u16)readRegByte(ERXRDPT+1))<<8);
	  erxwrpt = readRegByte(ERXWRPT) + (((u16)readRegByte(ERXWRPT+1))<<8);
	  erxst   = readRegByte(ERXST) + (((u16)readRegByte(ERXST+1))<<8);
	  erxnd   = readRegByte(ERXND) + (((u16)readRegByte(ERXND+1))<<8);
	  mamxfll = readRegByte(MAMXFL) + (((u16)readRegByte(MAMXFL+1))<<8);
	  printf("ERXRDPT ERXWRPT ERXST ERXND MAMXFL\n");
	  printf("%04x    %04x    %04x  %04x  %04x\n",
	        erxrdpt,erxwrpt,erxst,erxnd,mamxfll);

	  macon1 = readRegByte(MACON1);/*  + (((u16)readRegByte(MACON1+1))<<8); */
	  macon3 = readRegByte(MACON3);/*  + (((u16)readRegByte(MACON1+3))<<8); */
	  macon4 = readRegByte(MACON4);/*  + (((u16)readRegByte(MACON1+4))<<8); */
	  printf("MACON1 MACON3 MACON4\n");
	  printf("%02x     %02x     %02x\n",
	        macon1,macon3,macon4);
	  printf("MAADR\n");
	  printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
	  	(int)readRegByte(MAADR5),
	  	(int)readRegByte(MAADR4),
	  	(int)readRegByte(MAADR3),
	  	(int)readRegByte(MAADR2),
	  	(int)readRegByte(MAADR1),
	  	(int)readRegByte(MAADR0) );

	  val = readRegByte(EIR);
#endif
}

#if 0
/*  Contributed by Alex M. Based on code from: http://blog.derouineau.fr */
/*                   /2011/07/putting-enc28j60-ethernet-controler-in-sleep-mode/ */
void enc28j60_power_down( void ) 
{
    writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_RXEN);
    while(readRegByte(ESTAT) & ESTAT_RXBUSY);
    while(readRegByte(ECON1) & ECON1_TXRTS);
    writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_VRPS);
    writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PWRSV);
}

void enc28j60_power_up( void ) 
{
    writeOp(ENC28J60_BIT_FIELD_CLR, ECON2, ECON2_PWRSV);
    while(!readRegByte(ESTAT) & ESTAT_CLKRDY);
    writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

uint8_t enc28j60_do_BIST ( void ) 
{
	#define RANDOM_FILL		0b0000
	#define ADDRESS_FILL	0b0100
	#define PATTERN_SHIFT	0b1000
	#define RANDOM_RACE		0b1100

/*  init	 */
    spi_disable_eth();
    
    writeOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
    _delay_ms(2); /*  errata B7/2 */
    while (!readOp(ENC28J60_READ_CTRL_REG, ESTAT) & ESTAT_CLKRDY) ;

	/*  now we can start the memory test */
	uint16_t macResult;
	uint16_t bitsResult;

	/*  clear some of the registers registers */
    writeRegByte(ECON1, 0);
	writeReg(EDMAST, 0);
	
	/*  Set up necessary pointers for the DMA to calculate over the entire memory */
	writeReg(EDMAND, 0x1FFFu);
	writeReg(ERXND, 0x1FFFu);

	/*  Enable Test Mode and do an Address Fill */
	SetBank(EBSTCON);
	writeRegByte(EBSTCON, EBSTCON_TME | EBSTCON_BISTST | ADDRESS_FILL);
	
	/*  wait for BISTST to be reset, only after that are we actually ready to */
	/*  start the test */
	/*  this was undocumented :( */
    while (readOp(ENC28J60_READ_CTRL_REG, EBSTCON) & EBSTCON_BISTST);
	writeOp(ENC28J60_BIT_FIELD_CLR, EBSTCON, EBSTCON_TME);

	/*  now start the actual reading an calculating the checksum until the end is */
	/*  reached */
	writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_DMAST | ECON1_CSUMEN);
	SetBank(EDMACS);
	while(readOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_DMAST);
	macResult = readReg(EDMACS);
	bitsResult = readReg(EBSTCS);
	/*  Compare the results */
	/*  0xF807 should always be generated in Address fill mode */
	if ((macResult != bitsResult) || (bitsResult != 0xF807)) {
		return 0;
	}
	/*  reset test flag */
	writeOp(ENC28J60_BIT_FIELD_CLR, EBSTCON, EBSTCON_TME);
	
	
	/*  Now start the BIST with random data test, and also keep on swapping the */
	/*  DMA/BIST memory ports. */
	writeRegByte(EBSTSD, 0b10101010 /* | millis()*/);
	writeRegByte(EBSTCON, EBSTCON_TME | EBSTCON_PSEL | EBSTCON_BISTST | RANDOM_FILL);
						 
						 
	/*  wait for BISTST to be reset, only after that are we actually ready to */
	/*  start the test */
	/*  this was undocumented :( */
    while (readOp(ENC28J60_READ_CTRL_REG, EBSTCON) & EBSTCON_BISTST);
	writeOp(ENC28J60_BIT_FIELD_CLR, EBSTCON, EBSTCON_TME);
	
	
	/*  now start the actual reading an calculating the checksum until the end is */
	/*  reached */
	writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_DMAST | ECON1_CSUMEN);
	SetBank(EDMACS);
	while(readOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_DMAST);

	macResult = readReg(EDMACS);
	bitsResult = readReg(EBSTCS);
	/*  The checksum should be equal  */
	return macResult == bitsResult;
}
#endif


