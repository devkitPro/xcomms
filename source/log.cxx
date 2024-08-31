#include <stdio.h>
#include <stdarg.h>
#include <FL/Fl_Window.H>

#include "log.h"

Fl_Text_Display *LogWindow=(Fl_Text_Display *)0;

//static char StatusBuffer[256];

//-----------------------------------------------------------------------------
void LogChanged_cb(int nPos, int nInserted, int nDeleted,int, const char*, void* v) {
//-----------------------------------------------------------------------------
	int foundPos, ret;

//	Fl_Window *w = (Fl_Window*)v;
	ret = LogWindow->buffer()->findchar_backward(nPos+nInserted, '\n', &foundPos);

	int linelength = (nPos + nInserted) - foundPos;
//	snprintf(StatusBuffer,256,"%d %d %d\0",foundPos,ret,linelength);
	if (linelength >= 80) Log("\n");
//	w->label(StatusBuffer);

}

//-----------------------------------------------------------------------------
void Log(const char *Text, ...)
//-----------------------------------------------------------------------------
{
	char	line[256];

	va_list ap;

	va_start(ap, Text);
	vsnprintf( line, 256, Text, ap );
	va_end(ap);

	LogWindow->buffer()->append(line);

	while(LogWindow->move_down());

	LogWindow->show_insert_position();
}

