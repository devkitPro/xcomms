//-----------------------------------------------------------------------------
#ifndef _xboo_h_
#define _xboo_h_
//-----------------------------------------------------------------------------


typedef struct ROM
{
	unsigned short	*GBArom;
	int		length;
} ROMimage;

//-----------------------------------------------------------------------------
unsigned int		DelayExchange32(unsigned int Data);

int		NormalBoot(ROMimage *image);

void	SetVerify(char flag);
char	GetVerify();

void	SetBurst(char flag);
char	GetBurst();

void	SetDelay(char count);
char	GetDelay();

void XbooInit();

#ifdef __cplusplus
extern "C" {
#endif

void	ResetGBA();
void	InitPort();
void	PortDelay(int delay);
void	InitCRC();
int		ReadSOState();
void	XbooSend32(unsigned int Data);

unsigned int	XbooExchange32(unsigned int Data);
unsigned short	CalcCRC(unsigned int data);

#ifdef __cplusplus
}
#endif

//-----------------------------------------------------------------------------
#endif // _xboo_h_
//-----------------------------------------------------------------------------
