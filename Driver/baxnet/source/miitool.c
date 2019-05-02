/*******************************************************************************************/
/**                                                                                       **/
/**  miitool.c                                                                            **/
/**  SANA-II extension for access to Ethernet PHY registers over MII interface            **/
/**                                                                                       **/
/**  Copyright (c) 2019, Henryk Richter, Timm S. Müller                                   **/
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
/** Acknowledgements:                                                                     **/
/**  text and definitions in "media" table and "miitool.h" file originate from mii-tool.c **/
/**  written/copyright 1997-2000 by Donald Becker                                         **/
/*******************************************************************************************/
/* 
 Notes:
  - make sure you have an up-to-date devices/sana2devqueryext.h
  - make sure you have devices/newstyle.h
  - compilation
     sc  miitool.c link INCLUDEDIR=sc:netinclude
     gcc miitool.c -Igg:netinclude -o miitool
  - if you don`t have the Roadshow SDK installed, just comment "RSHOW" out to build
    without auto-detection code
*/

#define RSHOW

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <devices/newstyle.h>
#include <devices/sana2.h>
#include <devices/sana2devqueryext.h>
#include "miitool.h"
#ifdef RSHOW
#include <libraries/bsdsocket.h>
#include <proto/bsdsocket.h>
#endif


#ifdef __GNUC__
struct UtilityBase *UtilityBase;
static USHORT Utility_MY = 0;
#endif

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
 BYTE   *auto_devname; /* device name, if auto-detected */
};




void end_prog( struct prgdata *prg )
{
	if( prg->auto_devname )
		FreeVec( prg->auto_devname );

	if( prg->sana2devio )
	{
		if( prg->devvalid )
			CloseDevice( (struct IORequest *)prg->sana2devio );
		DeleteIORequest( (struct IORequest *)prg->sana2devio );
	}
	if( prg->sana2devport )
		DeleteMsgPort( prg->sana2devport );

#ifdef __GNUC__
	if( (UtilityBase) && (Utility_MY) )
	{
		CloseLibrary( (struct Library*)UtilityBase );
		Utility_MY  = 0;
		UtilityBase = NULL;
	}
#endif /* __GNUC__ */

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

#ifdef __GNUC__
	if( !UtilityBase )
	{
		UtilityBase = (struct UtilityBase*)OpenLibrary("utility.library",37);
		Utility_MY  = 1;
	}
#endif /* __GNUC__ */
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
} mediatypes[] = {
    /* these are kept in this order for the dump loop decision about best mode */
    { "10baseT-HD",     10,MII_AN_10BASET_HD },
    { "10baseT-FD",     10,MII_AN_10BASET_FD },
    { "100baseTx-HD",   12,MII_AN_100BASETX_HD },
    { "100baseTx-FD",   12,MII_AN_100BASETX_FD },
    { "100baseT4",       9,MII_AN_100BASET4 },
    /* generic mode names where only the speed matters */
    { "100baseTx",       9,MII_AN_100BASETX_FD | MII_AN_100BASETX_HD },
    { "10baseT",         7,MII_AN_10BASET_FD | MII_AN_10BASET_HD },
    { "100bTx",          6,MII_AN_100BASETX_FD | MII_AN_100BASETX_HD },
    { "10bT",            4,MII_AN_10BASET_FD | MII_AN_10BASET_HD },
};
#define NMEDIA (sizeof(mediatypes)/sizeof(mediatypes[0]))


void dump_mii_media( USHORT parms, BOOL bestonly )
{
	LONG i;
	
	for( i=9 ; i >= 5 ; i-- )
	{
		if( parms & (1<<i) )
		{
			Printf(" %s",(ULONG)mediatypes[i-5].name);
			if( bestonly )
				break;
		}
	}

	if( parms & (1<<10) )
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
			if( Strnicmp( mediatypes[i].name, s, mediatypes[i].namelen ) == 0 )
				break;
		if( i==NMEDIA )
			goto err;

		s += mediatypes[i].namelen;
		res |= mediatypes[i].value;

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
	Printf("Invalid media specification %s\n",(ULONG)parameters);
	Printf("Valid Arguments:\n");
	for( i=0 ; i < NMEDIA ; i++ )
		Printf("%s\n",(ULONG)mediatypes[i].name);
	return -1;
}

/*
  note: this code assumes that "mii" has the register contents in network
        byte ordering. Also, the registers themselves shall be consecutive
        in memory.
*/
int dump_mii( struct S2DevQueryMIIParam *mii, LONG length, LONG verbose )
{
	USHORT bmcr,bmsr,selfad,curlink;
	LONG i;

	bmcr    = mii[MII_BMCR].Content;
	bmsr    = mii[MII_BMSR].Content;
	selfad  = mii[MII_ANAR].Content;
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
		    if( selfad & curlink )
		    {
		    	/* if( curlink & MII_AN_ACK ) */
		    		Printf("negotiated ");
		    	/* else	Printf("no autonegotiation,"); */
		    	dump_mii_media( selfad & curlink, 1 );
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
	           (bmcr & MII_BMCR_100MBIT) ? (ULONG)"100" : (ULONG)"10",
	           (bmcr & MII_BMCR_DUPLEX) ? (ULONG)"full" : (ULONG)"half" );
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
		   (bmcr & MII_BMCR_100MBIT) ? (ULONG)"100" : (ULONG)"10",
		   (bmcr & MII_BMCR_DUPLEX) ? (ULONG)"full" : (ULONG)"half");
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
	  dump_mii_media( selfad, 0 );
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

#ifdef RSHOW
/* return string length without closing 0 */
LONG myStrLen( BYTE *str )
{
 LONG ret = 0;
 
 if( str )
 { 
	 while( *str++ )
 		ret++;
 }

 return ret;
}
/* copy string, append 0 */
void myStrNCpy( BYTE *dst, BYTE*src, LONG bytes )
{
 if( !dst )
 	return;

 if( src )
 {
 	while( (bytes--) && (*src != 0) )
 	{ 
 	  *dst++ = *src++;
 	}
 }
 *dst = 0;
 
}


/* find active interface and its device in RoadShow (optional) */
/* "which" index of Ethernet adapter 0,1,2,...                 */
/* "unit" is the output unit                                   */
BYTE *find_active_interface(LONG which, LONG *unit )
{
 struct Library *SocketBase;
 BYTE *ret = NULL;
 struct List * interface_list = NULL;
 struct Node *n;
 BYTE *dev = NULL;
 LONG hw = -1;
 
 SocketBase = OpenLibrary("bsdsocket.library",4);
 if( !SocketBase )
 {
 	Printf("Cannot open bsdsocket.library version 4 for device auto-detection\n");
 	return NULL;
 }
 
 do
 {
   LONG have_iface = 0;
 	
   if(  !(SocketBaseTags(SBTM_GETREF(SBTC_HAVE_INTERFACE_API), &have_iface, TAG_DONE) == 0)
      ||(!have_iface) )
   {
    Printf("RoadShow-style interface API not available in IP stack\n");
    break;
   }
   
   interface_list = ObtainInterfaceList();
   if( !interface_list )
   {
   
   	break;
   }
   for( n=interface_list->lh_Head ; n->ln_Succ != NULL ; n=n->ln_Succ )
   {
    /* Printf("Interface %s\n",n->ln_Name); */
    /* find Ethernet interface */
    if(!QueryInterfaceTags( n->ln_Name, IFQ_HardwareType, &hw, 
                                        IFQ_DeviceName, &dev,
                                        IFQ_DeviceUnit, unit ) )
    	continue;

    /* Printf("device %s hwtype %ld\n",dev,hw); */

    if( hw == S2WireType_Ethernet )
    	break;
   }

   if( hw == S2WireType_Ethernet )
   {
	if( dev )
	{
 		ret = AllocVec( myStrLen( dev ), 0 );
 		if( ret )
 			myStrNCpy( ret, dev, myStrLen(dev) );
	}
   }
 }
 while(0);

 if( interface_list )
 	ReleaseInterfaceList(interface_list);

 CloseLibrary(SocketBase);

 return ret;
}
#endif


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

  unit = (prg->opts.unit) ? *prg->opts.unit : 0;		

  if( !prg->opts.devname )
  {
#ifdef RSHOW
      LONG _unit;
      prg->auto_devname = find_active_interface( 0, &_unit );
      if( prg->auto_devname )
      {
       unit = _unit;
       name = prg->auto_devname;
      }
      else
      {
        Printf("cannot auto-determine device, please use DEVICE=... as argument\n");
	end_prog(prg);
      	return res;
      }
#else
      name = "enc624j6net.device";
      Printf("device auto-detect not compiled in, now trying default %s\n",name);
#endif  
      /* end_prog(prg);
      return res; */
  }
  else
      name = prg->opts.devname;

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
		/* read BMSR a second time */
		query_mii( prg, mii+MII_BMSR, MII_BMSR, MII_BMSR );
		res = RETURN_OK;
	}
	else
	{
		Printf("Sana2DeviceQueryExtension for MII access not present\n");
		res = RETURN_WARN;
	}
  }

  Printf("Device:  %s\n",(ULONG)name);
  /* general info */
  if( parm_vendorname.Actual != 0 )
  {
	vendorname[parm_vendorname.Actual-1] = 0;
	Printf("Vendor:  %s\n",(ULONG)vendorname);
  }
  if( parm_productname.Actual != 0 )
  {
	productname[parm_productname.Actual-1] = 0;
	Printf("Product: %s\n",(ULONG)productname);
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
		/* command or query ? */
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

