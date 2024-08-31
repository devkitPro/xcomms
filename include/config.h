//-----------------------------------------------------------------------------
#ifndef _config_h_
#define _config_h_
//-----------------------------------------------------------------------------

#define WIN_MIN_WIDTH	256
#define WIN_MIN_HEIGHT	256
//-----------------------------------------------------------------------------

typedef struct
{
	unsigned int header;
	short	Port;
	char	verify ,burst ,delay;
	int		x, y;
	int		width, height;
	char	headerfix;
	char	MBV2;
} cfg;

extern cfg XcommsCfg;

void ReadConfigFile(const char *file);
void WriteConfigFile(const char *file);
//-----------------------------------------------------------------------------
#endif //_config_h_
//-----------------------------------------------------------------------------
