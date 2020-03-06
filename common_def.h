#ifndef _COMMON_DEF_H
#define _COMMON_DEF_H

#include "mbed.h"

//-------------------------//
// project
//-------------------------//
// slc: smart lighting
#define x_PROJECT_SLC
#define x_PROJECT_SLC_FTM

#ifdef _PROJECT_SLC
#define x_PROJECT_SLC_COSTDOWN					// costdown SLC, no microchip, has power meter, relay, and pwm
#endif

// spm: smart power meter
#define x_PROJECT_SPM
#define _PROJECT_SPM_FTM

#ifdef _PROJECT_SPM
#define _PROJECT_SPM_FCU_INCODE					// FCU process in main code
#endif

// else: testing
#define x_ECHO_TEST_ENABLE

//-------------------------//
// firmware version
//-------------------------//
#define AAEON_FW_VERSION_MASTER     			10
#define AAEON_FW_VERSION_SLAVE      			2
#define AAEON_FW_VERSION_SUB					0

// 9.9:  modify os_reboot with modem reset option
// 9.10: remove hw reset in CellularExt_PreInitModule() when fail, and ignore hw reset for UBLOX-R410M (RESET_N can power down module only)
// 9.11: enlarge main-thread timeout 10->15mins, reduce wait time after modem sw reset to 5secs
//       printf rtc before/after sync
//       enlarge timer thread stack size 2k->3k
// 9.12: printf CellularExt_PreInitModule when entering function
// 9.13: send "AT+CEREG=2" in the beginning
// 9.14: remove 9.13, reduce time thread stack 3k->2k
// 9.15: mark some printf log in SlcLwm2mData to preven memory not enough
//       modify printf log in main
//       show resource subscribed status while waiting
// 9.16: test code to add dns server for ntp client, need more verify, mark now
// 10.0: create feature _PROJECT_SPM_FCU_INCODE and ftm for smart power meter
// 10.1: remove SlcCtrl to prevent SlcApp task overflow
// 10.2: enable feature _PROJECT_SPM_FCU_INCODE

//-------------------------//
// behavior feature
//-------------------------//
#define _MBEDCLOUDCLIENT_PAUSERESUME			// call pause for idle, resume for upload
												// undefine for calling register and deregister
#define _NO_DISCONNECT_WHEN_FW_UPDATING			// don't deregister and disconnect network when updating fw

#define x_TIMEZONE

#define x_OS_REBOOT_WITH_MODEM_HW_RESET
#define _OS_REBOOT_WITH_MODEM_SW_RESET

#define x_INIT_FAIL_WITH_MODEM_HW_RESET


//-------------------------//
// hardware feature
//-------------------------//
// data store
#define x_NO_CONFIGDATA_STORAGE					// for without sd card and spi nor-flash
#define x_SD_CARD								// define for connect to sd card for EVK, else for spi nor-flash for aaeon board

// network
#define x_ETHERNET								// define for ethernet, else cellular module
#define x_DEVICE_QUECTEL_BC95					// if define "CELLULAR_DEVICE=QUECTEL_BC95" in mbed_app.json
												// and define "PAL_UDP_MTU_SIZE=1358" in mbed_app.json
#define x_DEVICE_QUECTEL_BG96					// if define "CELLULAR_DEVICE=QUECTEL_BG96" in mbed_app.json
												// and define "PAL_UDP_MTU_SIZE=1358" in mbed_app.json
#define _DEVICE_UBLOX_SARA_R410M				// if define "CELLULAR_DEVICE=UBLOX_AT" in mbed_app.json
												// and define "UBLOX_SARA_R410M=1" in mbed_app.json
												// and define "PAL_UDP_MTU_SIZE=1024" in mbed_app.json
												// and define "mbed-client.sn-coap-max-blockwise-payload-size": 512 in mbed_app.json


#if defined(_DEVICE_QUECTEL_BC95) || defined(_DEVICE_UBLOX_SARA_R410M)
#define x_CELLULAR_DEVICE_BAND28				// specify support band 28 only
#define _CELLULAR_DEVICE_BAND3_8_28				// specify support band 3,8,28 only
#define x_CELLULAR_DEVICE_BANDFULL				// support full band
#endif

#ifdef _DEVICE_QUECTEL_BG96
#define x_CELLULAR_DEVICE_NBIOT					// config BG96 for NBIOT only if defined
#define x_CELLULAR_DEVICE_CATM1					// config BG96 for CatM1 only if defined
#define x_CELLULAR_DEVICE_AUTO					// config BG96 for auto mode if defined
#endif


//-------------------------//
// special operation
//-------------------------//
#define OP_REBOOT								44033	// 0xAC01
#define OP_CONFIG_FACTORY_RESET					48129	// 0xBC01
#define OP_KCM_FACTORY_RESET					56321	// 0xDC01

//-------------------------//
// date and time
//-------------------------//
#define NTP_DATETIME_BASE_YEAR					1900
#define NTP_DATETIME_BASE_MONTH					1

#define DATETIME_THRESHOLD_YEAR					2018	// RTC year should be larger than this value, and that can be verified as real time year
#define IF_REAL_RTC(year)						(year > DATETIME_THRESHOLD_YEAR)? true:false

//-------------------------//
// module specific
//-------------------------//
typedef enum _disconnect_type {
	DISCONNECT_JUSTNETWORK,						// just close network
	DISCONNECT_RAI,								// close network and send RAI flag
	DISCONNECT_DETACH							// close network and detach
} _DisconnectType;

typedef enum _modem_reset_type {
	MODEM_RESET_NONE,
	MODEM_RESET_SW,
	MODEM_RESET_HW
} _ModemResetType;


#if defined(_OS_REBOOT_WITH_MODEM_HW_RESET)
	#define DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG	MODEM_RESET_HW
#elif defined(_OS_REBOOT_WITH_MODEM_SW_RESET)
	#define DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG	MODEM_RESET_SW
#else
	#define DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG	MODEM_RESET_NONE
#endif

#ifdef _INIT_FAIL_WITH_MODEM_HW_RESET
	#define DEFAULT_INIT_FAIL_WITH_MODEM_RESET_CONFIG	MODEM_RESET_HW
#else
	#define DEFAULT_INIT_FAIL_WITH_MODEM_RESET_CONFIG	MODEM_RESET_NONE
#endif

//-------------------------//
// etc
//-------------------------//
#define THREAD_WATCHDOG_TIMEOUTTIME_SEC			480
#define MAINTHREAD_WATCHDOG_TIMEOUTTIME_SEC		900 //600

//-------------------------//
// common structure
//-------------------------//
typedef struct {
	uint16_t Y;
	uint8_t M;
	uint8_t D;
	uint8_t WD;
	uint8_t h;
	uint8_t m;
	uint8_t s;
} _DateTimeStruct;

typedef enum _net_state {
	NETSTATE_DISCONNECTED,
	NETSTATE_CONNECTING,
	NETSTATE_CONNECTED,
	NETSTATE_RAIDISCONNECTED,
	NETSTATE_DETACHDISCONNECTED,
	NETSTATE_ERROR,
	NETSTATE_ERROR_REBOOT
} _NetState;

//-------------------------//
// function type declaration
//-------------------------//
typedef void (*_Resource_Update_fxnType)(int idx, void *pval1, void *pval2);

typedef void (*_VoidCb)(void);

typedef void (*_VoidVoidCb)(void *);


//-------------------------//
// common function
//-------------------------//
void Pelion_ToDeregister(_DisconnectType DisconnectType);
void Pelion_ToRestart(void);
bool Pelion_IsRegistered(void);
bool Pelion_IsPaused(void);
_NetState Network_GetState(void);
void OS_Reboot(_ModemResetType reset_modem_type);


void RTC_PrintRealTime(void);
void RTC_GetRealTime(_DateTimeStruct *rtcdatetime);
time_t RTC_GetRealTimeStamp(void);
time_t RTC_GetUnixTimeStamp(void);
void RTC_TimeStampToRealTime(_DateTimeStruct *rtcdatetime, time_t timestamp);

char *Pelion_GetDeviceID(void);

void Main_PrintHeapStack(const char *tagstr);


// backup in here for mbed_app.json
// macro:
//"MBED_HEAP_STATS_ENABLED=1",
//"MBED_STACK_STATS_ENABLED=1",
//"MBED_MEM_TRACING_ENABLED=1"


#endif
	
