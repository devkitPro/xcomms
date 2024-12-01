#include	"xcomms_cmd.h"
#include	"log.h"
#include	"xboo.h"
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<unistd.h>
#ifdef __WIN32__
#include	<windows.h>
#endif

enum { WAITING, PRINTING, GET_FILENAME, GET_FILEMODE, FILE_WRITE, FILE_READ};
int ConsoleState = WAITING;
int ReadLength = 0;

extern volatile char gba_booted;
int SOState	= 0;
int LastSO	= 0;

#define MAX_FILES 127

char StringBuffer[256];
char FileMode[] = "wb";
char *NamePtr;
int OpenFiles = 0;

typedef struct { FILE* handle; char name[256]; } FileList;
FileList Files[MAX_FILES];

static int console_kbhit = 0;
static unsigned int last_keytext;



//-----------------------------------------------------------------------------
void console_keypress( const char *keytext)
//-----------------------------------------------------------------------------
{
	console_kbhit = -1;
	last_keytext = keytext[0];
}

//-----------------------------------------------------------------------------
int console_keytext()
//-----------------------------------------------------------------------------
{
	unsigned int tmp = last_keytext;
	console_kbhit = 0;
	last_keytext = 0;
	return tmp;
}

//-----------------------------------------------------------------------------
int OpenLocalFile(char *Name, char* Mode)
//-----------------------------------------------------------------------------
{
	int i;

	if (OpenFiles>= MAX_FILES) return -1;

	for (i=0; i<MAX_FILES; i++)
	{
		if (Files[i].handle == NULL) break;
	}

	Files[i].handle = fopen(StringBuffer,FileMode);

	if (Files[OpenFiles].handle == NULL)
	{
		i = -1;
	}
	else
	{
		strcpy(Files[i].name,StringBuffer);
		OpenFiles++;
	}

	return i;

}

//-----------------------------------------------------------------------------
void InitFiles()
//-----------------------------------------------------------------------------
{
	int i;
	for (i=0; i<MAX_FILES; i++)
	{
		Files[i].handle = NULL;
	}
}


//-----------------------------------------------------------------------------
void CloseAllFiles()
//-----------------------------------------------------------------------------
{
	int i;
	for (i=0; i<OpenFiles; i++)
	{
		if (Files[i].handle) fclose(Files[i].handle);
	}

	OpenFiles=0;
}


//-----------------------------------------------------------------------------
void GBAConsole()
//-----------------------------------------------------------------------------
{
	unsigned int cmd,letters, handle;
	unsigned char code;

	unsigned char *read_buffer, *read_ptr;

	unsigned int i, seek_offset, seek_origin, read_count, read_size, read_length;

	unsigned int data;

#ifdef __WIN32__
	Sleep(10);
#endif

	if (gba_booted)
	{

		SOState = ReadSOState();

		if (SOState == 0)
		{
			data = XbooExchange32(0);

			cmd = data & 0xffffff00;
			code = data & 0xff;

			switch(ConsoleState)
			{
			//-------------------------------------------------------------
			case GET_FILEMODE:
				FileMode[0] = ((unsigned char*)(&data))[0];
				FileMode[1] = ((unsigned char*)(&data))[1];


  				for( i = 0; i < strlen(StringBuffer); i++ )
  				{
					if( StringBuffer[i] == '\\' ) StringBuffer[i] = '/';
				}


				Log("File %s, mode %s\n", StringBuffer, FileMode);

				handle = OpenLocalFile(StringBuffer, FileMode);

				while (ReadSOState());
				XbooExchange32(handle);
				ConsoleState = WAITING;
				break;
			//-------------------------------------------------------------
			case GET_FILENAME:
				ReadLength -= 4;
				letters = 4;
				if (ReadLength<0)
				{
					letters += ReadLength;
					ReadLength = 0;
				}
				for (i=0; i<letters; i++)
				{
					*(NamePtr++) = ((unsigned char*)(&data))[i];
				}
				if (ReadLength == 0)
				{
					ConsoleState = GET_FILEMODE;
					*(NamePtr++)=0;
				}
				break;
			//-------------------------------------------------------------
			case PRINTING:
				ReadLength -= 4;
				letters = 4;
				if (ReadLength<0)
				{
					letters += ReadLength;
					ReadLength = 0;
				}
				for (i=0; i<letters; i++)
				{
					code = ((unsigned char*)(&data))[i];
					{
						Log("%c",code);
					}
				}
				if (ReadLength == 0) ConsoleState = WAITING;
				break;
			//-------------------------------------------------------------
			case WAITING:
				switch (cmd)
				{
				//---------------------------------------------------------
				case KBHIT_CMD:
					data = console_kbhit;
					while (ReadSOState());
					XbooSend32(data);
					break;
				//---------------------------------------------------------
				case GETCH_CMD:
					data = console_keytext();
					while (ReadSOState());
					XbooSend32(data);
					break;
				//---------------------------------------------------------
				case DPUTC_CMD:
					{
						Log("%c",code);
					}
					break;
				//---------------------------------------------------------
				case PRINT_CMD:
					ReadLength = code;
					ConsoleState = PRINTING;
					break;
				//---------------------------------------------------------
				case FOPEN_CMD:
					ReadLength = code;
					ConsoleState = GET_FILENAME;
					NamePtr = StringBuffer;
					break;
				//---------------------------------------------------------
				case FCLOSE_CMD:
					if (code < MAX_FILES && Files[code].handle)
					{
						fclose(Files[code].handle);
						Log("Closed handle %d - %s\n",code,Files[code].name);
						Files[code].handle = NULL;
						OpenFiles--;
					}
					break;
				//---------------------------------------------------------
				case FPUTC_CMD:

					while(ReadSOState());
					data = XbooExchange32(0);

					if (code < MAX_FILES && Files[code].handle)
					{
						fputc(data,Files[code].handle);
					}
					break;
				//---------------------------------------------------------
				case FGETC_CMD:
					if (code < MAX_FILES && Files[code].handle)
					{
						data = fgetc(Files[code].handle);
					}

					while (ReadSOState());
					XbooSend32(data);


					break;
				//---------------------------------------------------------
				case FSEEK_CMD:

					while (ReadSOState());
					seek_offset = XbooExchange32(data);
					while (ReadSOState());
					seek_origin = XbooExchange32(data);

					if (code < MAX_FILES && Files[code].handle)
					{
						fseek(Files[code].handle,seek_offset,seek_origin);
						Log("Seek on %s: Offset %d Origin %d\n",Files[code].name, seek_offset, seek_origin);
					}
					break;
				//---------------------------------------------------------
				case FTELL_CMD:
					data = 0xffffffff;
					if (code < MAX_FILES && Files[code].handle)
					{
						data = ftell(Files[code].handle);
						Log("Ftell on %s\n",Files[code].name);
					} else {
						data = 0;
					}

					while (ReadSOState());
					XbooSend32(data);

					break;
				//---------------------------------------------------------
				case FREAD_CMD:
					while (ReadSOState());
					read_size	= XbooExchange32(0);
					while (ReadSOState());
					read_count  = XbooExchange32(0);

					read_length = read_size * read_count;

					read_buffer = (unsigned char *)malloc( ((read_length)+3)&-4);
					read_ptr = read_buffer;

					if (code < MAX_FILES && Files[code].handle)
					{
						fread(read_buffer,read_size,read_count,Files[code].handle);
						Log("Read %d bytes from %s\n",read_length,Files[code].name);
					}

					read_length = (read_length +3 ) & -4;
					for (i=0; i<read_length; i++)
					{

						((unsigned char*)(&data))[i&3]=*(read_ptr++);

						if ( (i&3) == 3)
						{
							while (ReadSOState());
							XbooSend32(data);
						}

					}

					free(read_buffer);
					break;
				//---------------------------------------------------------
				case FWRITE_CMD:
					while (ReadSOState());
					read_size	= XbooExchange32(0);
					while (ReadSOState());
					read_count  = XbooExchange32(0);

					read_length = read_size * read_count;

					read_buffer = (unsigned char *)malloc( ((read_length)+3)&-4);
					read_ptr = read_buffer;


					read_length = (read_length +3 ) & -4;
					for (i=0; i<read_length; i++)
					{
						if ( (i&3) == 0)
						{
							while (ReadSOState());
							data = XbooExchange32(data);
						}

						*(read_ptr++) = ((unsigned char*)(&data))[i&3];

					}

					if (code < MAX_FILES && Files[code].handle)
					{
						fwrite(read_buffer,read_size,read_count,Files[code].handle);
						Log("Wrote %d bytes to %s\n",read_size*read_count,Files[code].name);
					}

					free(read_buffer);
					break;
				}
				break;

			}

		}

	}
}

