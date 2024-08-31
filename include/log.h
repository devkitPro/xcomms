//-----------------------------------------------------------------------------
#ifndef _log_h
#define _log_h
//-----------------------------------------------------------------------------

#include <FL/Fl_Text_Display.H>

extern Fl_Text_Display *LogWindow;

void Log(const char *Text, ...);
void LogChanged_cb(int nPos, int nInserted, int nDeleted,int, const char*, void* v);

//-----------------------------------------------------------------------------
#endif // _log_h
//-----------------------------------------------------------------------------
