#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>

#include	<xboo.h>
#include <parport.h>
#include	<config.h>

//-----------------------------------------------------------------------------
cfg XcommsCfg;

FILE *CfgHnd;

//-----------------------------------------------------------------------------
void ReadConfigFile(const char *file)
//-----------------------------------------------------------------------------
{
	memset ( &XcommsCfg, 0, sizeof(XcommsCfg));

	if ( (CfgHnd = fopen( file,"rb")) )
	{
		fread( &XcommsCfg, 1,sizeof(XcommsCfg), CfgHnd);
		fclose( CfgHnd);
	}
	if (!CfgHnd || XcommsCfg.header != 'XCFG')
	{
		XcommsCfg.header 	= 'XCFG';
		XcommsCfg.burst		= 1;
		XcommsCfg.delay		= 1;
		XcommsCfg.verify	= 1;
		XcommsCfg.headerfix	= 1;
		XcommsCfg.Port		= 0x378;
		XcommsCfg.x			= 0;
		XcommsCfg.y			= 0;
		XcommsCfg.width		= WIN_MIN_WIDTH;
		XcommsCfg.height	= WIN_MIN_HEIGHT;
		XcommsCfg.MBV2		= 0;
		
	}

	SetDelay(XcommsCfg.delay);
	SetVerify(XcommsCfg.verify);
	SetBurst(XcommsCfg.burst);
	SetPortAddress(XcommsCfg.Port);
}

//-----------------------------------------------------------------------------
void WriteConfigFile(const char* file)
//-----------------------------------------------------------------------------
{
	XcommsCfg.burst		= GetBurst();
	XcommsCfg.delay		= GetDelay();
	XcommsCfg.verify	= GetVerify();
	XcommsCfg.Port		= GetPortAddress();

	if ( (CfgHnd = fopen( file,"wb")))
	{
		fwrite( &XcommsCfg, sizeof(XcommsCfg), 1, CfgHnd);
		fclose( CfgHnd );
	}
}
