#include <mbv2.h>
#include <parport.h>

#ifdef __WIN32__
	#include <windows.h>
#else
	#include <time.h>
	#include <unistd.h>
#endif

#include <log.h>


//-----------------------------------------------------------------------------
void outp ( short address, unsigned char value) {
//-----------------------------------------------------------------------------

	asm ( "\n\t"
				"movw	%[address],%%dx\n\t"
				"movb	%[value],%%al\n\t"
				"out	%%al, %%dx\n\t"
				:
				: [address] "q" (address), [value] "q" (value)
			);
	
} 

//-----------------------------------------------------------------------------
int inp ( short address) {
//-----------------------------------------------------------------------------

	int value;
	
	asm ( "\n\t"
				"movw	%[address],%%dx\n\t"
				"in	%%dx, %%al\n\t"
				"movsbl	%%al,%[value]\n\t"
				: [value] "=q" (value)
				: [address] "q" (address)
			);
	
	return value;

} 

//-----------------------------------------------------------------------------
void TimerDelay2 (int delay) {
//-----------------------------------------------------------------------------

	unsigned long delayms = delay * 9;
	
#ifdef __linux__
	
	sleep(delayms);
#else
	MMRESULT result = timeBeginPeriod( 1  ); // set timer resolution to 1ms
	
	DWORD currentTime = timeGetTime();
	DWORD wantedTime = currentTime + delayms;

	if ( wantedTime < currentTime) {
		while ( timeGetTime() > wantedTime ); 
	}
	
	while ( timeGetTime() < wantedTime);

	result = timeEndPeriod( 1  ); // reset timer resolution
#endif
}

//-----------------------------------------------------------------------------
// Delay for at least DELAY*18 milliseconds
//-----------------------------------------------------------------------------
void TimerDelay (int delay) {
//-----------------------------------------------------------------------------
	TimerDelay2 (delay+1);
}


short data_port;
short status_port;
short control_port;
short EPP_DataPort;
short ECPRegECR;
int MBV2Firmware;



//-----------------------------------------------------------------------------
void ResetMBV2() {
//-----------------------------------------------------------------------------

	int status = inp ( control_port);
	
	status &= 0xfb;
	outp (control_port, status);
	TimerDelay(1);
	status |= 4;
	outp (control_port, status);

}

//-----------------------------------------------------------------------------
void setMBV2mode(int mode) {
//-----------------------------------------------------------------------------
	outp (data_port, mode);
	ResetMBV2();
	TimerDelay(1);

}


//-----------------------------------------------------------------------------
bool DetectMBV2() {
//-----------------------------------------------------------------------------
	int i,j,hw0,hw1;
	
	bool found = false;

	Log("detecting MBV2\n");

	data_port = GetPortAddress();
	status_port = data_port + 1;
	control_port = data_port + 2;
	EPP_DataPort = data_port + 4;
	ECPRegECR = data_port + 0x402;

	setMBV2mode(MODE_CADETECT);

	outp (data_port, 0);
	TimerDelay(1);
	i = ((inp(status_port)) >> 3) & 0xf;
	outp (data_port, 1);
	TimerDelay2(1);
	j = ((inp(status_port)) >> 3) & 0xf;
	outp (data_port, 2);
	TimerDelay2(1);
	hw0 = (((inp(status_port)) >> 3) & 0xf);
	outp (data_port, 3);
	TimerDelay2(1);
	hw1 = (((inp(status_port)) >> 3) & 0xf);
	outp (data_port, 4);
	TimerDelay2(1);
	MBV2Firmware = (((inp(status_port)) >> 3) & 0xf) << 4;
	outp (data_port, 5);
	TimerDelay2(1);
	MBV2Firmware += (((inp(status_port)) >> 3) & 0xf);

	if ( (i != 0x0c) || (j != 0x0a) ) {
		Log("MBV2 hardware not found on selected port.\n");
		found = true;
	} else {

		Log("Parallel MBV2 cable found: hardware=v%d.%d, firmware=v%d\n", hw0,hw1,MBV2Firmware);
	}


		
	return found;
}
