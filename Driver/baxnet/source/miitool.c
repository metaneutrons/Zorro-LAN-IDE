/*******************************************************************************************/
/**                                                                                       **/
/**  miitool.c                                                                            **/
/**  SANA-II extension for access to Ethernet PHY registers over MII interface            **/
/**                                                                                       **/
/**  Copyright (c) 2019, Henryk Richter, Timm S. Müller                                   **/
/**  All rights reserved.                                                                 **/
/**                                                                                       **/
/**  Redistribution and use in source and binary forms, with or without                   **/
/**  modification, are permitted provided that the following conditions are met:          **/
/**  1. Redistributions of source code must retain the above copyright                    **/
/**     notice, this list of conditions and the following disclaimer.                     **/
/**  2. Redistributions in binary form must reproduce the above copyright                 **/
/**     notice, this list of conditions and the following disclaimer in the               **/
/**     documentation and/or other materials provided with the distribution.              **/
/**  3. All advertising materials mentioning features or use of this software             **/
/**     must display the following acknowledgement:                                       **/
/**     This product includes software developed by Henryk Richter.                       **/
/**  4. Neither the name of the Henryk Richter nor the                                    **/
/**     names of its contributors may be used to endorse or promote products              **/
/**     derived from this software without specific prior written permission.             **/
/**                                                                                       **/
/** THIS SOFTWARE IS PROVIDED BY Henryk Richter ''AS IS'' AND ANY                         **/
/** EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED             **/
/** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE                **/
/** DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY                    **/
/** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            **/
/** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;          **/
/** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND           **/
/** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT            **/
/** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         **/
/** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                          **/
/**                                                                                       **/
/*******************************************************************************************/
/* 
 Notes:
  - make sure you have an up-to-date devices/sana2devqueryext.h
  - make sure you have devices/newstyle.h
  - compilation
     sc miitool.c link

*/


#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <devices/newstyle.h>
#include <devices/sana2.h>
#include <devices/sana2devqueryext.h>
#include "miitool.h"


#define OPT_TEMPLATE "DEVICE=DEV/K,UNIT/K/N,VERBOSE=V/S,FORCEMEDIA/K,ADVERTISE/K,RENEG=RESTART/S,RESET/S,VERYVERBOSE=VV/S"
const char opt_template[] = OPT_TEMPLATE;
struct progopts {
	BYTE  *devname;
	LONG  *unit;       /* unit to open */
	ULONG verbose;     /* verbose mode enabled     */
	BYTE  *force;      /* force media type         */
	BYTE  *advertise;  /* advertise specific types */
	ULONG renegotiate; /* restart autonegotiation  */
	ULONG reset;       /* reset MII                */
	ULONG vverbose;    /* higher verbosity level   */
};

struct prgdata {
 struct MsgPort    *sana2devport;
 struct IOSana2Req *sana2devio;
 struct Device     *sana2dev;
 UBYTE  devvalid; 
 struct progopts opts;
};



void end_prog( struct prgdata *prg )
{
	if( prg->sana2devio )
	{
		if( prg->devvalid )
			CloseDevice( (struct IORequest *)prg->sana2devio );
		DeleteIORequest( (struct IORequest *)prg->sana2devio );
	}
	if( prg->sana2devport )
		DeleteMsgPort( prg->sana2devport );
}

/*
  note: call this only once at program startup
*/
LONG start_prog( struct prgdata *prg )
{
	SHORT i;
	UBYTE *t = (UBYTE*)prg;
	for( i=0 ; i < sizeof( struct prgdata ) ; i++ )
		*t++ = 0;

	prg->sana2devport = CreateMsgPort();
	if( !prg->sana2devport )
		goto err;

	prg->sana2devio = (struct IOSana2Req *) CreateIORequest(
	                  prg->sana2devport, sizeof(struct IOSana2Req));
	if( !prg->sana2devio )
		goto err;

	return 1;
err:
	end_prog( prg );
	return 0;
}


void clr_device( struct prgdata *prg )
{
	if( !prg->devvalid )
		return;
	prg->devvalid = 0;
	CloseDevice( (struct IORequest *)prg->sana2devio );
}


LONG get_device(struct prgdata *prg, BYTE *name, LONG unit )
{
	if( !name )
		return 0;
	if( prg->devvalid )	/* close previously opened device */
		clr_device( prg );

	prg->devvalid = OpenDevice(name,unit,(struct IORequest *)prg->sana2devio,0) == 0;
	if( !prg->devvalid )
	{
		Printf( "Cannot open %s unit %ld\n",(ULONG)name,unit );
		return 0;
	}
	return 1;
}

static BOOL findcmd(struct NSDeviceQueryResult *nsdqr, UWORD cmd)
{
	UWORD *p;
	for (p = nsdqr->SupportedCommands; (*p); ++p)
		if (*p == cmd)
			return TRUE;
	return FALSE;
}

/*
  verify NSD support, SANA-II type, Device Query extension on previously opened device
*/
LONG probe_device( struct prgdata *prg )
{
	struct IOStdReq *stdreq;
	struct NSDeviceQueryResult nsdqr;

	if( !prg )
		return 0;
	if( !prg->devvalid )
		return 0;

	stdreq = (struct IOStdReq *)prg->sana2devio;
	stdreq->io_Command = NSCMD_DEVICEQUERY;
	stdreq->io_Flags = 0;
	stdreq->io_Data = &nsdqr;
	stdreq->io_Length = sizeof nsdqr;
	if (DoIO((struct IORequest *) stdreq) != 0)
	{
		Printf("I/O error: no NewStyleDevice.\n");
		return 0;
	}
	
	if (nsdqr.DeviceType != NSDEVTYPE_SANA2)
	{
		Printf("Device is not a SANA-II device (type %ld, needed %ld\n).", (LONG)nsdqr.DeviceType, (LONG) NSDEVTYPE_SANA2);
		return 0;
	}

	if (!findcmd(&nsdqr, S2_DEVICEQUERYEXT))
	{
		Printf("Device does not support S2_DEVICEQUERYEXT.\n");
		return 0;
	}

	return 1;
}

const struct {
    BYTE	*name;
    UWORD	namelen;
    UWORD	value;
} media[] = {
    /* The order through 100baseT4 matches bits in the BMSR */
    { "10baseT-HD",     10,MII_AN_10BASET_HD },
    { "10baseT-FD",     10,MII_AN_10BASET_FD },
    { "100baseTx-HD",   12,MII_AN_100BASETX_HD },
    { "100baseTx-FD",   12,MII_AN_100BASETX_FD },
    { "100baseT4",       9,MII_AN_100BASET4 },
    { "100baseTx",       9,MII_AN_100BASETX_FD | MII_AN_100BASETX_HD },
    { "10baseT",         7,MII_AN_10BASET_FD | MII_AN_10BASET_HD },
    { "100bTx",          6,MII_AN_100BASETX_FD | MII_AN_100BASETX_HD },
    { "10bT",            4,MII_AN_10BASET_FD | MII_AN_10BASET_HD },
};
#define NMEDIA (sizeof(media)/sizeof(media[0]))

void dump_mii_media( USHORT par, BOOL bestonly )
{
	LONG i;
	
	par >>= 5; /* get upper bits */
	for( i=4 ; i >= 0 ; i-- )
	{
		if( par & (1<<i) )
		{
			Printf(" ");
			Printf("%s",(ULONG)media[i].name);
			if( bestonly )
				break;
		}
	}
	if( par & (1<<5) )
		Printf(" flow-control");
}

LONG parse_speedduplex( BYTE *parameters )
{
	LONG i,res = 0;
	BYTE *s;

	if( !parameters )
		return 0;

	s = parameters;
	while( *s != 0 )
	{
		for( i=0 ; i < NMEDIA ; i++ )
			if( Strnicmp( media[i].name, s, media[i].namelen ) == 0 )
				break;
		if( i==NMEDIA )
			goto err;

		s += media[i].namelen;
		res |= media[i].value;

		/* skip " " and/or "," */
		while( *s != 0 )
		{
			if( (*s==' ') || (*s==',') )
				s++;
			else
				break;
		}
	}
	
	return res;
err:	
	Printf("Invalid media specification %s\n",parameters);
	Printf("Valid Arguments:\n");
	for( i=0 ; i < NMEDIA ; i++ )
		Printf("%s\n",media[i].name);
	return -1;
}

/*
  note: this code assumes that "mii" has the register contents in natural 
        byte ordering. Also, the registers themselves shall be consecutive
        in memory.
*/
int dump_mii( struct S2DevQueryMIIParam *mii, LONG length, LONG verbose )
{
	USHORT bmcr,bmsr,advert,curlink;
	LONG i;

	bmcr    = mii[MII_BMCR].Content;
	bmsr    = mii[MII_BMSR].Content;
	advert  = mii[MII_ANAR].Content;
	curlink = mii[MII_ANLPAR].Content;

	if( bmcr == 0xffff )
	{
		Printf("No MII transceiver found\n");
		return 0;
	}

	if( bmcr & MII_BMCR_AN_ENA) 
	{
		if( bmsr & MII_BMSR_AN_COMPLETE)
		{
		    if( advert & curlink )
		    {
		    	if( curlink & MII_AN_ACK )
		    		Printf("negotiated ");
		    	else	Printf("no autonegotiation,");
		    	dump_mii_media( advert & curlink, 1 );
		    	Printf(", ");
		    }
		    else
		    {
			Printf("autonegotiation failed, ");
	    	    }
	    	}
		else 
		 if (bmcr & MII_BMCR_RESTART)
		 {
		    Printf("autonegotiation restarted, ");
		 }
	}
	else
	{
	    Printf("%s Mbit, %s duplex, ",
	           (bmcr & MII_BMCR_100MBIT) ? "100" : "10",
	           (bmcr & MII_BMCR_DUPLEX) ? "full" : "half" );
	}
	if( bmsr & MII_BMSR_LINK_VALID )
		Printf("link ok");
	else	Printf("no link");
	Printf("\n");
	
	if( verbose > 1 )
	{
		Printf("PHY Registers:");
		for( i=0 ; i < length/sizeof( struct S2DevQueryMIIParam ) ; i++ )
		{
			if(!(i%8))
				Printf("\n");
			if( mii[i].ID == S2_DEVQUERYEXT_MII_INVALID )
				Printf("N/A ");
			else
				Printf("%04lx ",(LONG)(mii[i].Content));
		}
		Printf("\n");
	}

	if( verbose )
	{
	  Printf(" vendor %02lx:%02lx:%02lx, model %ld rev %ld\n",
		   (LONG)mii[2].Content>>10, 
		   (LONG)(mii[2].Content>>2)&0xff,
		   (LONG)((mii[2].Content<<6)|(mii[3].Content>>10))&0xff,
		   (LONG)(mii[3].Content>>4)&0x3f,
		   (LONG)(mii[3].Content&0x0f)
		   );
	  Printf(" basic mode:   ");
	  if( bmcr & MII_BMCR_RESET )
	  	Printf("software reset, ");
	  if( bmcr & MII_BMCR_LOOPBACK )
		Printf("loopback, ");
	  if( bmcr & MII_BMCR_ISOLATE )
		Printf("isolate, ");
	  if( bmcr & MII_BMCR_COLTEST )
		Printf("collision test, ");
	  if( bmcr & MII_BMCR_AN_ENA )
	    Printf("autonegotiation enabled\n");
	  else
	    Printf("%s Mbit, %s duplex\n",
		   (bmcr & MII_BMCR_100MBIT) ? "100" : "10",
		   (bmcr & MII_BMCR_DUPLEX) ? "full" : "half");
	  Printf(" basic status: ");
	  if( bmsr & MII_BMSR_AN_COMPLETE )
	    Printf("autonegotiation complete, ");
	  else if( bmcr & MII_BMCR_RESTART )
	    Printf("autonegotiation restarted, ");
	  if( bmsr & MII_BMSR_REMOTE_FAULT )
	    Printf(" remote fault, ");
	  Printf( (bmsr & MII_BMSR_LINK_VALID) ? "link ok" : "no link");
	  Printf("\n capabilities:");
	  dump_mii_media( bmsr >> 6, 0 );
	  Printf("\n advertising: ");
	  dump_mii_media( advert, 0 );
	  if( curlink & MII_AN_ABILITY_MASK )
	  {
	  	Printf("\n link partner:");
		dump_mii_media( curlink, 0 );
	  }
	  Printf("\n");
	}

	return 1;
}


/*
  Query consecutive set of MII registers from start to end (inclusive)
*/
LONG query_mii( struct prgdata *prg, struct S2DevQueryMIIParam *mii, LONG start, LONG end )
{
	struct S2DevQueryMIIParam *miitmp;
	struct S2DevQueryExtParam parm_mii;
	struct TagItem tags[2];
	LONG i;

	/* MII specific */
	parm_mii.Data           = (APTR)mii;
	parm_mii.Length         = (end-start+1)*sizeof(struct S2DevQueryMIIParam);
	parm_mii.Actual         = 0;

	for( i=start,miitmp=mii ; i <= end ; i++,miitmp++ ) /* MII standard register set: 8 */
	{
		miitmp->ID = i;
	}

	tags[i=0].ti_Tag  = S2_DevQueryExtGetMII;
	tags[i++].ti_Data = (ULONG) &parm_mii;
	tags[i  ].ti_Tag  = TAG_DONE;

	prg->sana2devio->ios2_Req.io_Command = S2_DEVICEQUERYEXT;
	prg->sana2devio->ios2_Data = tags;
	prg->sana2devio->ios2_DataLength = sizeof(tags);
		
	if( DoIO((struct IORequest *) prg->sana2devio ) != 0 )
		return 0;
	else
		return parm_mii.Actual;
}

LONG write_mii( struct prgdata *prg, struct S2DevQueryMIIParam *mii, LONG num )
{
	struct S2DevQueryExtParam parm_mii;
	struct TagItem tags[2];
	LONG i;

	/* MII specific */
	parm_mii.Data           = (APTR)mii;
	parm_mii.Length         = (num)*sizeof(struct S2DevQueryMIIParam);
	parm_mii.Actual         = 0;

	tags[i=0].ti_Tag  = S2_DevQueryExtSetMII;
	tags[i++].ti_Data = (ULONG) &parm_mii;
	tags[i  ].ti_Tag  = TAG_DONE;

	prg->sana2devio->ios2_Req.io_Command = S2_DEVICEQUERYEXT;
	prg->sana2devio->ios2_Data = tags;
	prg->sana2devio->ios2_DataLength = sizeof(tags);
		
	if( DoIO((struct IORequest *) prg->sana2devio ) != 0 )
		return 0;
	else
		return parm_mii.Actual;
}

/* 8 basic registers, we poll 16 out of curiosity */
#define MII_REGNUM 16
/* two spare registers for write mode */
#define MII_SPARE  2

int main(int argc, char **argv)
{
  struct prgdata _prg,*prg=&_prg;
  int res = RETURN_FAIL;
  char vendorname[128], productname[128];
  struct S2DevQueryExtParam parm_productname;
  struct S2DevQueryExtParam parm_vendorname;
  struct S2DevQueryExtParam parm_mii;
  struct S2DevQueryMIIParam mii[MII_REGNUM+MII_SPARE];
  struct TagItem tags[3];
  int    i;
  struct RDArgs *rdargs;
  BYTE   *name;
  LONG   unit,autoneg,force;

  if( !start_prog(prg) )
      return res;

  if( !(rdargs = ReadArgs( (char*)opt_template,(LONG*)&prg->opts,NULL) ) )
  {
      Printf("Argument error\n");
      end_prog(prg);
      return res;
  }

  if( !prg->opts.devname )
  {
      name = "enc624j6net.device";
      Printf("TODO: auto-detect, now trying default %s\n",name);
      /* end_prog(prg);
      return res; */
  }
  else
      name = prg->opts.devname;

  unit = (prg->opts.unit) ? *prg->opts.unit : 0;		
  if( prg->opts.vverbose )
      prg->opts.verbose = 2;
  else
      if( prg->opts.verbose )
          prg->opts.verbose = 1;

  /* parameter check */
  if( ( (prg->opts.force) && (prg->opts.renegotiate) ) ||
      ( (prg->opts.force) && (prg->opts.advertise)   ) 
    )
  {
  	Printf("Error: Forced and Autonegotiation parameter mismatch\n");
  	end_prog(prg);
  	return RETURN_FAIL;
  }

  /* convert text to binary mask */
  autoneg = parse_speedduplex( prg->opts.advertise );
  force   = parse_speedduplex( prg->opts.force );
  if( (autoneg|force) < 0 )
  {
  	end_prog(prg);
  	return RETURN_FAIL;
  }
        
  if( get_device( prg, name , unit ) )
  {
	if( probe_device( prg ) )
		res = RETURN_OK;
  }
  if( res != RETURN_OK )
  {
	end_prog(prg);
	return res;
  }

  /* device open, query extension present, now query it */
  {
	/* general information */
	parm_vendorname.Data    = vendorname;
	parm_vendorname.Length  = sizeof(vendorname);
	parm_vendorname.Actual  = 0;
	parm_productname.Data   = productname;
	parm_productname.Length = sizeof(productname);
	parm_productname.Actual = 0;
#if 1
	tags[i=0].ti_Tag  = S2_DevQueryExtVendorName;
	tags[i++].ti_Data = (ULONG) &parm_vendorname;
	tags[i  ].ti_Tag  = S2_DevQueryExtProductName;
	tags[i++].ti_Data = (ULONG) &parm_productname;
	tags[i  ].ti_Tag = TAG_DONE;

	prg->sana2devio->ios2_Req.io_Command = S2_DEVICEQUERYEXT;
	prg->sana2devio->ios2_Data = tags;
	prg->sana2devio->ios2_DataLength = sizeof(tags);
		
	DoIO( (struct IORequest *)prg->sana2devio );

	/* separate query subroutine in here for MII, though not 
	   absolutely necessary (MII is queried more than once) */
	parm_mii.Actual = query_mii( prg, mii, 0, 15 );
	if( parm_mii.Actual )
	{
		/* read BMSR twice */
		 query_mii( prg, mii+MII_BMSR, MII_BMSR, MII_BMSR );
		res = RETURN_OK;
	}
	else
		res = RETURN_WARN;
#else
		/* MII specific */
		parm_mii.Data           = (APTR)mii;
		parm_mii.Length         = sizeof(mii);
		parm_mii.Actual         = 0;

		for( i=0 ; i < 16 ; i++ ) /* MII standard register set: 8 */
		{
			mii[i].ID = i;	/* mii[i].Content = 0; */
		}
		mii[16].ID = MII_BMSR;  /* read BMSR twice */

		tags[i=0].ti_Tag  = S2_DevQueryExtVendorName;
		tags[i++].ti_Data = (ULONG) &parm_vendorname;
		tags[i  ].ti_Tag  = S2_DevQueryExtProductName;
		tags[i++].ti_Data = (ULONG) &parm_productname;
		tags[i  ].ti_Tag  = S2_DevQueryExtGetMII;
		tags[i++].ti_Data = (ULONG) &parm_mii;
		tags[i  ].ti_Tag = TAG_DONE;

		prg->sana2devio->ios2_Req.io_Command = S2_DEVICEQUERYEXT;
		prg->sana2devio->ios2_Data = tags;
		prg->sana2devio->ios2_DataLength = sizeof tags;
		
		if( DoIO((struct IORequest *) prg->sana2devio ) != 0 )
			res = RETURN_WARN;
		else	res = RETURN_OK;

		mii[MII_BMSR].Content = mii[16].Content;  /* read BMSR twice */
#endif
  }

  Printf("Device:  %s\n",name);
  /* general info */
  if( parm_vendorname.Actual != 0 )
  {
	vendorname[parm_vendorname.Actual-1] = 0;
	Printf("Vendor:  %s\n",vendorname);
  }
  if( parm_productname.Actual != 0 )
  {
	productname[parm_productname.Actual-1] = 0;
	Printf("Product: %s\n",productname);
  }

  /* after query, see what we've got */
  if( res == RETURN_OK )
  {
	/* MII data */
	if( parm_mii.Actual == 0 )
	{
		Printf("Unable to query MII data.\n");
	}
	else
	{
		if(    !(prg->opts.renegotiate) && !(prg->opts.reset)
		    && !(prg->opts.advertise)   && !(prg->opts.force)
		  )
		{
		   Printf("Active Parameters:\n");
		   dump_mii( mii, parm_mii.Actual, prg->opts.verbose );
		}
	}

	if( prg->opts.reset )
	{
		Printf("resetting the transceiver...\n");
		mii[MII_REGNUM].ID = MII_BMCR;
		mii[MII_REGNUM].Content = MII_BMCR_RESET;
		write_mii( prg, &mii[MII_REGNUM], 1 );
	}

	if( prg->opts.advertise )
	{
		mii[MII_REGNUM].ID = MII_ANAR;
		mii[MII_REGNUM].Content = autoneg|1;
		write_mii( prg, &mii[MII_REGNUM], 1 );
		prg->opts.renegotiate = 1;
	}

	if( prg->opts.renegotiate )
	{
		Printf("restarting autonegotiation...\n");

		mii[MII_REGNUM].ID = MII_BMCR;
		mii[MII_REGNUM].Content = 0x0000;
		write_mii( prg, &mii[MII_REGNUM], 1 );

		mii[MII_REGNUM].ID = MII_BMCR;
		mii[MII_REGNUM].Content = MII_BMCR_AN_ENA|MII_BMCR_RESTART;
		write_mii( prg, &mii[MII_REGNUM], 1 );
	}

	if( prg->opts.force )
	{
		UWORD bmcr = 0;
		if( force & (MII_AN_100BASETX_FD|MII_AN_100BASETX_HD))
	 	   bmcr |= MII_BMCR_100MBIT;
		if( force & (MII_AN_100BASETX_FD|MII_AN_10BASET_FD))
		   bmcr |= MII_BMCR_DUPLEX;
		
		mii[MII_REGNUM].ID = MII_BMCR;
		mii[MII_REGNUM].Content = bmcr;
		write_mii( prg, &mii[MII_REGNUM], 1 );
	}
  }


  end_prog(prg);
  return res;
}
