#include <xboo.h>
#include <log.h>
#include <parport.h>

//-----------------------------------------------------------------------------
char	verify		= 1;
char	burst		= 1;
char	delay		= 1;
char	gba_booted	= 0;
//-----------------------------------------------------------------------------
void SetVerify( char flag)	{ verify = flag; }
char GetVerify()			{ return verify; }
//-----------------------------------------------------------------------------
void SetBurst( char flag)	{ burst = flag; }
char GetBurst()				{ return burst; }
//-----------------------------------------------------------------------------
void SetDelay( char count)	{ delay = count; }
char GetDelay()				{ return delay; }
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void XbooInit()
//-----------------------------------------------------------------------------
{
	InitPort();
	Log("Using Port Address %0x\n",GetPortAddress());
	Log("Port Initialisation\n");
}

//-----------------------------------------------------------------------------
unsigned int DelayExchange32( unsigned int Data)
//-----------------------------------------------------------------------------
{
	unsigned int ret;
	ret = XbooExchange32(Data);
	PortDelay(delay);

	return ret;
}

//-----------------------------------------------------------------------------
void DelaySend32(unsigned int data)
//-----------------------------------------------------------------------------
{
	XbooSend32(data);
	PortDelay(delay);
}


//-----------------------------------------------------------------------------
unsigned int HandShake(unsigned short Send, unsigned char Receive)
//-----------------------------------------------------------------------------
{
	int				i = 0;
	unsigned int	Reply;

	do {
		PortDelay(100);
		Reply = XbooExchange32( Send );
		if (i++ >1000) break;
	} while ( (Reply>>24) != Receive);

	return Reply;
}


//-----------------------------------------------------------------------------
int NormalBoot(ROMimage *image)
//-----------------------------------------------------------------------------
{
	unsigned int	i;
	unsigned short	*HeaderData;
	unsigned int	*MainData, data, seed, Reply;
	unsigned int	RomLength;
	unsigned int	Offset;

	unsigned short	Key1FromSlave,Key2FromSlave, KeyConfirm;
	unsigned short	CRCtmp;

	unsigned short	KeyFromMaster = 0xd1; //0xc1; //0xd1; //0xe1;

	gba_booted =0;

	Log("\nReset and requesting Normal boot.\n");
	ResetGBA();

	//-------------------------------------------------------------------------
	// Length must be even multiple of 16
	// and greater than 448
	//-------------------------------------------------------------------------
	RomLength = (image->length+0x0f) & -16;
	if (RomLength < 0x01c0)	RomLength = 0x01c0;

	Reply = HandShake(0x6202,0x72);		// attention code
	if ( (Reply>>24) != 0x72 )
	{
		Log("No response from GBA\n");
		return 0;
	}

	Log("Sending Header ... ");
	Reply = DelayExchange32( 0x6102 );	// header code

	HeaderData = image->GBArom;

	for (i=0x60;i>0;i--)
	{
		data	= *(HeaderData++);
		Reply	= DelayExchange32( data );
		if ( (Reply >>24) != i )
			break;
	}

	if (i)
	{
		Log("Failed\n");
		return (0);
	}

	Log("Success\n");

	Reply = DelayExchange32( 0x6200 );	// header finished
	Reply = DelayExchange32( 0x6202 );


	KeyConfirm = 0x6300 | KeyFromMaster;

	Reply = HandShake( KeyConfirm, 0x73);

	Key1FromSlave = (unsigned short)((Reply>>16) & 0xff);
	KeyConfirm = ((Key1FromSlave + 0x0f) & 0xff) | 0x6400;

	Reply = HandShake( KeyConfirm, 0x73);

	seed = 0x0FFFF0000 | ( Key1FromSlave <<8) | KeyFromMaster;

	PortDelay(0x110e);

	Reply = DelayExchange32( ((RomLength) >> 2) - 0x64 );

	Key2FromSlave = (unsigned short)((Reply >>16) &0x00ff);

	InitCRC();

	MainData = (unsigned int *)HeaderData;


	Log("Sending Main Data ... ");

	Offset = 192;


	RomLength = (RomLength-192) >> 2;

	for (i=0; i< RomLength; i++)
	{
		data = *(MainData++);
		CRCtmp = CalcCRC(data);

		seed = 0x6F646573 * seed + 1 ;

		data ^= seed;

		data ^= (0x0FE000000 - Offset) ^ 0x43202F2F;

		if (verify)
		{
			Reply = DelayExchange32(data);
		}
		else
		{
			DelaySend32(data);
		}

		if ( verify && (Reply>>16 != (Offset &0xffff)) )
			break;

		Offset += 4;

	}

	if (i != RomLength)
	{
		Log("Failed\n");
		return (0);
	}

	Log("Success\n");

	Log("Final Confirmation ...");


	Reply = DelayExchange32(0x65);

	i = 0;
	do
	{
		Reply = DelayExchange32( 0x65);
		PortDelay(200);
		if (i++ > 1000) break;
	} while ( (Reply>>16) == 0x74);

	if ( (Reply>>16) != 0x75 )
	{
		Log("failed\nprobably bad header\n");
		return 0;
	}

	Reply = DelayExchange32(0x66);

	data = 0xffff0000 | ((Key2FromSlave & 0x00ff) <<8) | (KeyConfirm & 0x00ff);

	CRCtmp = CalcCRC(data);

	Reply = DelayExchange32(CRCtmp);

	if ((Reply >>16) != CRCtmp)
	{
		Log("Failed final CRC check\n");
		return (0);
	}

	Log("Success\n");

	gba_booted = 1;

	return 0;
}


