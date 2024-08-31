//-----------------------------------------------------------------------------
#ifndef _mbv2_h
#define _mbv2_h
//-----------------------------------------------------------------------------

#define MODE_LOWPOWER 0
#define MODE_GBARESET 1
#define MODE_MULTIPLR 2
#define MODE_CADETECT 4
#define MODE_UARTSLOW 5
#define MODE_UARTFAST 6
#define MODE_EPPTEST 7
#define MODE_INDIRECT 8
#define MODE_NC256_R4 9
#define MODE_NC256_R1 10
#define MODE_EPPNC256_R4 11
#define MODE_EPPNC256_R1 12
#define MODE_EPPNC400_R1 13
#define MODE_EPPNC700_R1 14
#define MODE_EPPNC1000_R1 15
#define MODE_EPPNC1000_R4 16

#define MODE_NCGBACLK 72
#define MODE_NCPCFAST 73
#define MODE_NCPCMED  74
#define MODE_SERIAL   255

#define GBA_IDLE 0x41
#define GBA_NOA_FLASH 0x42
#define GBA_VISOLY_OLDER 0x43
#define GBA_VISOLY_NEWER 0x45
#define GBA_WAITING_FOR_DATA 0x46
#define GBA_ERROR_IN_DATA 0x47
#define GBA_ERROR_SLOT_EMPTY 0x48
#define GBA_BUSY 0x49
#define GBA_GARBAGE_COMMAND 0x4a
#define GBA_CRC_GOOD 0x4b
#define GBA_CRC_BAD 0x4c
#define GBA_EXECUTING_CART 0x4d

bool DetectMBV2();


//-----------------------------------------------------------------------------
#endif // _mbv2_h
//-----------------------------------------------------------------------------
