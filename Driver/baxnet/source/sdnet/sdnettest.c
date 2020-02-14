/*
  test program for SDNet 

  author: Henryk Richter <henryk.richter@gmx.net>

  Runs a number of essential operations on ENC28J60
  modules connected to the SD slot of Vampire V2 
  (500/600) cards.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <exec/exec.h>
#include <dos/dos.h>
#include <clib/macros.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "enc28j60.h"
#include "enc28j60l.h" /* import lowlevel asm functions */
/* #include "crc32.h" */

#include "vampuuid.h"
/*#define __NOLIBBASE__*/
#include <proto/vampire.h>
#include <vampire/vampire.h>
#define INVALID_SDNET_UNIT 0x33

/* delay after changing SPI speed in us */
#define SPEED_DELAY	120

struct Library *VampireBase;

/* */
/*u08 myprog = "sdnettest";*/

/* OUI = Commodore :-) */
u08 macaddr[6] = { 0x00,0x80,0x10,0x0b,0x0a,0xff };

#if 1
#define testlen 300
/* issue a DHCP request to increase the likelihood of received frames throughout the test */
u08 testframe[testlen] = {
/* Ethernet(14) */
	0xff,0xff,0xff,0xff,0xff,0xff, /* broadcast */
	0x00,0x80,0x10,0x0b,0x0a,0xff, /* own MAC   */
	0x08,0x00, /* Type IPv4 */
/* IPv4(20) */
	/* IPv4 */
	0x45,0x00,     /* VER/IHL, DSCP */
	0x01,0x1e,     /* was 0d */ /* length 273 = 245+20 */
	0xA0,0x0A,     /* id */
	0x00,0x00,     /* flags, fragment offset */
	0xff,          /* TTL (255 for DHCP) */
	0x11,          /* UDP */
	0x00,0x00,     /* header checksum */
	0,0,0,0,       /* SRC IP */
	255,255,255,255,/* dest IP */
/* UDP(8) */
	/* UDP */
	0x00,0x44,	/* src port 68 */
	0x00,0x43,	/* dest port 67 */
	0x01,0x0a,	/* length including UDP header FIXME: CHANGE when payload changes */
	0x00,0x00,      /* checksum */
/* BOOTP(245) */
	/* BOOTP/DHCP */
	0x01,	/* BOOT Request */
	0x01,	/* HW Type Ethernet */
	0x06,	/* HW Address length */
	0x00,	/* hops */
	0xde,0xad,0xbe,0xef, /* transaction ID */
	0x00,0x02, /* seconds elapsed */
	 /* mandatory bootp stuff */
	0x00,0x00,	/* flags */
	0x00,0x00,0x00,0x00, /* client IP */
	0x00,0x00,0x00,0x00, /* next server IP */
	0x00,0x00,0x00,0x00, /* relay agent IP */
	0x00,0x00,0x00,0x00, /* some more stuff */
	0x00,0x80,0x10,0x0b,0x0a,0xff, /* own MAC */
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, /* 10 bytes padding */
	 /* server name (64x0) */
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 /* boot file name (128x0) */
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* DHCP magic cookie */
	0x63,0x82,0x53,0x63,
	/* DHCP options */
	0x35,0x01,0x01, /* DHCP discover */
	0x37,0x04,0x01,0x03,0x06,0x0f, /* we want mask,router,DNS,Domain Name */
	0x3d,0x06,      /* client identifier (3c would be vendor class) */
	0,
	's','d','n','e','t',
	0xff, /* end */
/* padding */
/*	0x00,0x00,0x00 */
};
#else
#define testlen 98
u08 testframe[testlen] = {
	0xff,0xff,0xff,0xff,0xff,0xff, /* broadcast */
	0x00,0x80,0x10,0x0b,0x0a,0xff, /* own MAC   */
	0x08,0x00, /* Type IPv4 */
	/* */
	0x45,0x00,     /* VER, DSCP */
	0x00,0x54,     /* length */
	0xA0,0x0A,     /* id */
	0x00,0x00,     /* flags, fragment offset */
	0x40,          /* TTL */
	0x01,          /* ICMP */
	0x00,0x00,     /* header checksum */
	192,168,10,11, /* SRC IP */
	192,168,10,255,/* dest IP */
	/* */
	0x08,0x00,     /* Type, Code = ICMP Echo */
	0x00,0x00,     /* Checksum */
	0xDE,0xAD,     /* Identifier */
	0x00,0x01,     /* SeqNum */

	0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
	0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
	0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
	0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
	0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
	0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
	0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
}; /* 98 Bytes */
#endif

unsigned char recvbuf[1530];

ULONG vres = (ULONG)-1;
ULONG doquit = 0;

u16 ip_checksum( u08 *, LONG );
u16 udp_checksum( u08 *, LONG );

/* SAS/C abort override */
int CXBRK(void )
{
/*	doquit = 1; */
	return 0;
}

void __regargs __chkabort(void);
void __regargs __chkabort(void) 
{ 
 if( SetSignal( 0, SIGBREAKF_CTRL_C ) & SIGBREAKF_CTRL_C )
 {
	doquit = 1;
	printf("got ctrl-c\n");
 }
}

/*
  global cleanup
*/
void cleanup(void)
{
	if( VampireBase )
		V_FreeExpansionPort( vres );
	printf("cleaning up\n");		
}


/*
  communications test: try different clock dividers and poll revision
*/
ULONG sdnt_commtest( void )
{
	ULONG ret = 0;  /* def: fail */
	int speed = 10; /* try different SPI clock dividers */
	int wait;
	u08 rev,rdy=0;

	/*  soft reset cpu */
	enc28j60_SetSPISpeed( (ULONG)speed );
	enc28j60l_UMinDelay( SPEED_DELAY ); /* wait a little, */
	enc28j60l_WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	/* enc28j60l_WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	enc28j60l_WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	enc28j60l_WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);*/
	enc28j60l_UMinDelay( 4000 ); /* wait 4ms */
	wait = 0xfff;
	while( wait-- > 0 )
	{
		rdy = ESTAT_CLKRDY & enc28j60l_ReadOp(ENC28J60_READ_CTRL_REG, ESTAT );
		if( rdy )
			wait = 0;		
	}
	enc28j60l_UMinDelay( 20 ); /* wait a little, */

	if( !rdy )
	{
		printf("Fail. Chip doesn`t respond ready.\n");
	}
	else
	{
		rdy = enc28j60l_ReadRegByte(EREVID);
		if( rdy > 5 ) rdy++;
	}
	
	while( speed >= 0 )
	{
		enc28j60_SetSPISpeed( (ULONG)speed );
		enc28j60l_UMinDelay( SPEED_DELAY ); /* wait a little, */
		
		enc28j60l_ReadRegByte(EREVID);
		rev = enc28j60l_ReadRegByte(EREVID);
		if( rev > 5 ) rev++;

		if( rdy ) /* best case: chip reported "ready" */
		{
		 if( rev != rdy )
		 {
		 	if( speed >= 2 )
		 		printf("Fail. Different revision register reported by chip (%d vs %d at speed %d)\n",rdy,rev,speed);
		 	else
		 		printf("Note. Speed %d does not work properly with your card (but that was expected)\n",speed);
		 }
		 else
		 {
		 	printf("Ok.   Revision %d reported at speed %d\n",rev,speed);
		 	ret |= (1<<speed);
		 }
		}
		else /* chip was not "ready" but we`re stubborn */
		{
			if( (rev > 0) && (rev < 0x10 ) )
			{
				printf("OK. got Chip rev %d at speed %d\n",rev,speed);
				ret |= (1<<speed);
			}
			else	
				printf("Fail. Cannot read chip rev at speed %d\n",speed);
		}
		speed--;
	}
	
	if( !ret )
	{
		printf("Fail. No response from revision command,\n");
		printf("      Check ENC28J60 power supply and cabling.\n");
	}
	else	printf("Ok.   Speed success mask %x\n",ret );
	
	return ret;
}

/*
  communications test: try different clock dividers and copy memory forth and back

  input: bit mask of clock dividers to test (between 0 and 31)
*/
#define TXSTART_INIT        0x1A00  /*  start of TX buffer, room for 1 packet */
#define TXSTOP_INIT         0x1FFF  /*  end of TX buffer */
ULONG sdnt_memtest( ULONG speeds, ULONG testsize )
{
	u08 *membuf;
	int i,j,res;
	
	membuf = AllocVec( testsize*2, MEMF_ANY );
	if( !membuf )
	{
		printf("Fail. Cannot allocate %d Bytes Memory\n",testsize*2);
		return 0;
	}
	/*
	  fill buffer with something nonzero
	*/
	for( i=0 ; i < testsize ; i++ )
	{
		membuf[i] = (u08)( i ^ 0x5a );
	}

	/* try different speeds */
	for( i=31; i >= 0 ; i-- )
	{
	 if( speeds & (1<<i) )
	 {
		enc28j60_SetSPISpeed( (ULONG)i );
		enc28j60l_ReadRegByte(EREVID); /* safety */
		enc28j60l_UMinDelay( SPEED_DELAY ); /* wait 2 ms */
		enc28j60l_ReadRegByte(EREVID);

		enc28j60l_WriteBuffer(membuf,(int)testsize,TXSTART_INIT);
		
/*		enc28j60l_WriteReg( ERXRDPT, TXSTART_INIT ); */
		enc28j60l_WriteReg(ERDPT, TXSTART_INIT+1 );

		enc28j60l_ReadBuffer( membuf + testsize, (int)testsize );
		for( j=0, res=0 ; j < (testsize-1) ; j++ )
		{
			/*if( membuf[j] != membuf[j+testsize] ) 
				printf("%d vs %d\n",membuf[j], membuf[j+testsize] );*/

			res |= ( membuf[j] ^ membuf[j+testsize] );
		}
		if( res )
		{
		 printf("Fail. Memory transfer failure at speed %d\n",i);
		 speeds &= ~(1<<i);
		}
		else
		 printf("Ok.   Memory transfer success at speed %d\n",i);
	 }
	}

	FreeVec( membuf );

	if( !speeds )
	{
		printf("Fail. Memory transfer test failed. Couldn`t transfer data successfully.\n");
		printf("      Check ENC28J60 power supply and cabling.\n");
	}
	else	printf("Ok.   Speed success mask %x\n",speeds );

	return speeds;
}

#define TEST_MAC    (1<<30)
#define TEST_LINK   (1<<31)
#define TEST_TXERR  (1<<29)
#define TEST_RXERR  (1<<28)
#define TEST_HAVERX (1<<27)
int main( int argc, char **argv)
{
	u08 res;
	int runs;
	int ret = 0;
#ifdef PROTO_V2EXPNET
	ULONG portid = V_WIFIPORT;
#else
	ULONG portid = V_SDPORT;
#endif

	ULONG speeds;  /* clock divider mask */
	ULONG testres = 0; /* bit 31 is used for link status, bit 30 for correct MAC */
	ULONG clockdiv;

	printf("starting\n");

	atexit( cleanup );

	/*
	  verify vampire.resource presence
	*/
	if( VampireBase = OpenResource( V_VAMPIRENAME ) )
	{
		printf("Ok.  vampire.resource present\n");
		{
			APTR allocator = V_AllocExpansionPort( portid, argv[0] );
			if( allocator )
			{
				printf("Fail. Port seems to be allocated already by %s\n",allocator);
				printf("      Please shut down your IP stack and try again.\n");
				exit(1);
			}
			vres = portid;
			printf("Ok.  vampire.resource port %d allocated\n",vres);
		}
	}
	else
	{
		printf("Fail. vampire.resource not found.\n");
		printf("      Please install Vampire Gold 2.7 or later core,\n");
		printf("      flash an updated ROM or\n");
		printf("      loadmodule vampire.resource\n");
#ifdef PROTO_V2EXPNET
		printf("Note. v2expeth.device will NOT work without the resource\n");
#else
		printf("Note. sdnet.device will NOT work without the resource\n");
#endif
		printf("I`ll continue the test without resource allocation...\n");
	}

	/*
	  communications test, try to read HW version
	*/
	speeds = sdnt_commtest();
	if( !speeds )
	{
		printf("Exiting\n");
		ret  = 2;
		goto end;
	}

	/*
	  transfer test from/to HW memory 
	*/
	speeds = sdnt_memtest( speeds, 1536 );
	if( !speeds )
	{
		ret = 3;
		goto end;
	}

	/*
	  initialize Chip
	*/
	res = enc28j60_init( macaddr, PIO_INIT_MULTI_CAST|PIO_INIT_BROAD_CAST );
	/* res = enc28j60_init( macaddr, PIO_INIT_PROMISC ); */

	if( res != PIO_OK )
	{
	 printf("init error %d\n",res);
	 ret = 4;
	 goto end;
	}

	{
		u08 readmac[6];
		int i,res;
		
		enc28j60_getMAC( readmac );
		/*readmac[5]=0;*/
		
		for( i=0,res=0 ; i<6 ; i++ )
			res |= ( readmac[i] ^ macaddr[i] );
			
		if(res)
		{
			printf("Fail. Cannot read correct MAC address back: ");
					printf("got %02x.%02x.%02x.%02x.%02x.%02x wanted ",
					       readmac[0],readmac[1],readmac[2],
					       readmac[3],readmac[4],readmac[5]);
					printf("%02x.%02x.%02x.%02x.%02x.%02x\n",
					       macaddr[0],macaddr[1],macaddr[2],
					       macaddr[3],macaddr[4],macaddr[5]);
		
		}
		else
		{
			printf("OK.   init success. MAC address configured.\n");
			testres |= TEST_MAC;
		}
	}

	{
		unsigned char status;
		
		/* we checked the chip rev already... */
		/* enc28j60_status( PIO_STATUS_VERSION, &status ); printf("Chip Rev %d ",status); */
		
		enc28j60_status( PIO_STATUS_LINK_UP, &status );
		if( status != 1 )
			printf("Fail. Link status is %d. Check your Ethernet Switch and cabling.\n",status);
		else
		{
			printf("OK.   Link status is %d\n",status);
			testres |= TEST_LINK;
		}
	}

	/* set best supported speed */
	{

		for( clockdiv=0 ; clockdiv < 16 ; clockdiv++ )
		{
		 if( speeds & (1<<clockdiv) )
		 	break;
		}
		printf("      Setting best speed %d (i.e. fastest working SPI clock divider)\n",clockdiv);
		enc28j60_SetSPISpeed( clockdiv );
		enc28j60l_UMinDelay( SPEED_DELAY ); /* wait a little, */
	}

	printf("..... Sending 100 frames and log broadcasts/multicasts (~10 seconds) .....\n");
	printf("      (Whether something is received in the next 10 seconds depends\n");
	printf("       on which and how many devices are present+active in your network)\n");
	runs = 100;
	
	ip_checksum( testframe+14, testlen-14 );  /* only the first 20 bytes are relevant */
	udp_checksum( testframe+14, testlen-14 ); /* skip Ethernet header */
	
	while( (runs--) && (!doquit) )
	{
#if 1
		res = enc28j60_send( testframe, testlen );
		if( res != PIO_OK )
		{
			testres |= TEST_TXERR;
			printf("send error %d\n",res);
		}
#endif
		if(0)
		{
			unsigned char status;

			enc28j60_status( PIO_STATUS_LINK_UP, &status );
			printf("Link status is %d\n",status);
		}
		
		res = enc28j60_has_recv();
		/* printf("%d packets\n",res); */
		if( res )
		{
			unsigned short got_size;
			
			printf("%d packets\n",res);
			testres |= TEST_HAVERX;
			
/*			u08 enc28j60_recv(u08 *data, u16 max_size, u16 *got_size) */
			res = enc28j60_recv( recvbuf,1520,&got_size);
			if( res != PIO_IO_ERR )
			{
				/* printf("recv res %d\n",res); */
				if( got_size )
				{
				 unsigned int type = (((unsigned int)recvbuf[12])<<8) + (unsigned int)recvbuf[13];
				 if(1)/* if( type != 0x800 ) */
				 {
					printf("%d bytes\n",got_size);
					printf("%02x.%02x.%02x.%02x.%02x.%02x (%x)\n",
					       recvbuf[0],recvbuf[1],recvbuf[2],
					       recvbuf[3],recvbuf[4],recvbuf[5],
					       type );
				 }
				}
			}
		}
#if 0
		else
		{
			enc28j60_dump_regs();
		}
#endif
		Delay(5);
	}

	printf("Done. Verdict:\n");
	if( testres == ( TEST_HAVERX|TEST_LINK|TEST_MAC ) )
	{
#ifdef PROTO_V2EXPNET
		printf("      This test determined that your expansion net is quite likely in working shape.\n");
#else
		printf("      This test determined that your sdnet is quite likely in working shape.\n");
#endif
	}
	else
	{
		if( testres == ( TEST_LINK|TEST_MAC ) )
		{
			printf("      No reception while testing. Either you`ve got a quiet network or further testing might be needed.\n");
		}
		else
		{
			if( testres & (TEST_TXERR|TEST_RXERR) )
				printf("      RX/TX problems encountered.\n");
			if( !(testres & TEST_LINK) )
				printf("      Link is not up\n");
			if( !(testres & TEST_MAC) )
				printf("      MAC address not set correctly\n");
			if( testres & TEST_HAVERX )
			 	printf("      Some problems but at least we received data\n");
		}
	}

	if( clockdiv != 1 )
	{
		printf("Important: your SDNet device was not working in this test with clockdiv=1.\n");
		printf("           Please call\n");
		printf("            makedir ENVARC:Sana2\n");
#ifdef PROTO_V2EXPNET
		printf("            echo \"SPISPEED=%d\" >ENVARC:sana2/v2expeth.config\n",clockdiv);
#else
		printf("            echo \"SPISPEED=%d\" >ENVARC:sana2/sdnet.config\n",clockdiv);
#endif		
	}
end:
	return ret;
}

/* IP header checksum 
   input:  IP header
           length   (min. 20)
   output: checksum (also directly in IP header)
   
*/
u16 ip_checksum( u08 *buffer, LONG length )
{
	ULONG csum = 0;

	ULONG words,i;
	unsigned short *b2,*b3;

	if( length < 20 )
		return 0;	/* no checksum */

	/* IPv4 header */
	b2 = (unsigned short*)buffer;
	b2[5] = 0;
	words = ((ULONG)(buffer[0] & 0xf))<<1;	 /* IHL, def: 5 (32 bit words) */

	/* sum up */
	for( i=0,b3=b2 ; i < words ; i++ )
	{
	 csum += *b3++;
	}

	while( csum > 0xffff )
	{
		csum = (csum&0xffff)+(csum>>16);
	}
	csum = (~csum)&0xffff;

	b2[5] = csum;	/* store checksum */

	return (u16)csum;

}

/*
  calculate UDP checksum
  - buffer supplied should point to IP packet, where src/dest/protocol are filled in

  length is in octets, including IPv4 header and UDP header

  input:  IP packet including UDP header and content
  output: checksum (also directly in UDP header)
*/
u16 udp_checksum( u08 *buffer, LONG length )
{
	ULONG words,i;
	ULONG csum = 0;
	unsigned short *b2,*b3;

	if( length < 28 )
		return 0;	/* no checksum */

	/* IPv4 pseudoheader (see for UDP length below) */
	csum += *((unsigned short*)(&buffer[12])); /* src IP  */	
	csum += *((unsigned short*)(&buffer[14])); /* src IP  */	
	csum += *((unsigned short*)(&buffer[16])); /* dest IP */
	csum += *((unsigned short*)(&buffer[18])); /* dest IP */
	csum += (ULONG)buffer[9];  /* protocol */
	
	/* location of UDP header */
	words = ((ULONG)(buffer[0] & 0xf))<<2;	 /* IHL, def: 5 (32 bit words) */
	b2    = (unsigned short*)(&buffer[words]);
	
	b2[3] = 0;	/* clear checksum */
	b3    = b2;

	csum += b2[2]; /* UDP length */

	words = (length-words)>>1; /* total length - IP header */
	for( i=0 ; i<words ; i++ )
	{
		csum += (ULONG)*b2++;
	}
	
	if( length & 1 )
	{
		csum += (ULONG)*b2;
	}
	
	while( csum > 0xffff )
	{
		csum = (csum&0xffff)+(csum>>16);
	}
	csum = csum^0xffff;
	
	b3[3] =	(u16)csum;	/* store checksum */

	return (u16)csum;
}
