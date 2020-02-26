#ifndef _SLC_APPLICATION_H
#define _SLC_APPLICATION_H

#include "common_def.h"
#include "SlcControlData.h"
#include "SlcLwm2mData.h"
#include "common_def.h"

#define TIME_MIN											60
#define TIME_HOUR											3600
#define TIME_DAY											86400

#define x_CERTIFICATION_TEST								// for NCC test
#define x_NODEREG_KEEPNODEREG
//-----------------------------------------------------------------------------------//
// behavior defination
//-----------------------------------------------------------------------------------//
#ifdef _CERTIFICATION_TEST
#define TAG_VER												0xCA00

#define DEFAULT_KEEP_ONLINE									1
#define DEFAULT_NODE_REGISTER								true

#elif defined(_NODEREG_KEEPNODEREG)

#define TAG_VER												0xDA00

#define DEFAULT_KEEP_ONLINE									0
#define DEFAULT_NODE_REGISTER								true

#else

#define TAG_VER												0xAA00

#define DEFAULT_KEEP_ONLINE									0
#define DEFAULT_NODE_REGISTER								true

#endif

#define DEFAULT_CONFIG_VERSION								(16 | TAG_VER)
#define DEFAULT_STORE_NODE_REGISTERED						true

#define DEFAULT_POWERON_FIRST_UPLOAD_PROCEDURE_TIME_SEC				(30)
#define DEFAULT_POWERON_FIRST_UPLOAD_PROCEDURE_RANDOM_RANGE_SEC		(90)

#define REDUCE_UPLOAD_PERIOD_TIMES							12
#define DEFAULT_FIRST_UPLOAD_PROCEDURE_TIME_SEC				(2*TIME_MIN)	// offline time
#define DEFAULT_FIRST_UPLOAD_PROCEDURE_RANDOM_RANGE_SEC		(30)

#define DEFAULT_CYCLE_UPLOAD_PROCEDURE_TIME_SEC				(7*TIME_MIN) //(3*TIME_MIN)	// offline time
#define DEFAULT_CYCLE_UPLOAD_PROCEDURE_RANDOM_RANGE_SEC		(2*TIME_MIN)

#define DEFAULT_CYCLE_UPLOAD_NODEREGISTER_TIME_SEC			(5*TIME_MIN)
#define DEFAULT_CYCLE_UPLOAD_NODEREGISTER_RANDOM_RANGE_SEC	5

#define DEFAULT_CYCLE_UPLOAD_STATUS_TIME_SEC				(5*TIME_MIN)
#define DEFAULT_CYCLE_UPLOAD_STATUS_RANDOM_RANGE_SEC		7

#define DEFAULT_CYCLE_UPLOAD_DATA_TIME_SEC					(TIME_HOUR)
#define DEFAULT_CYCLE_UPLOAD_DATA_RANDOM_RANGE_SEC			45

#define DEFAULT_UPLOAD_RAMDON								true
#define DEFAULT_UPLOAD_DATA									true

#define CYCLE_UPLOAD_TIME_MIN_SEC							30


#ifdef _DEVICE_QUECTEL_BC95
#define DEFAULT_DISCONNECT_TYPE								DISCONNECT_RAI
#else
#define DEFAULT_DISCONNECT_TYPE								DISCONNECT_DETACH
#endif
#define DEFAULT_KEEP_ONLINE_TIME							(3*TIME_MIN)

#define DEFAULT_REG_WATCHDOG_TIMEOUT_SEC					(10*TIME_MIN*2 + 60)//(DEFAULT_CYCLE_UPLOAD_NODEREGISTER_TIME_SEC * 8)
															// NETWORK_TIMEOUT in AT_CellularContext.cpp is 30mins --> sophie modify to 10mins
															// retries for 2 times and buffer 60sec

#define DEFAULT_NODEREG_WATCHDOG_TIMEOUT_SEC				(10*TIME_MIN)

#define DEFAULT_SYS_WATCHDOG_TIMEOUT_SEC					(30*TIME_MIN) // 30 mins //(48*TIME_HOUR)	// 2days
#define DEFAULT_LIGHT_SENSOR_TIMEOUT_SEC					TIME_HOUR		// 1hr
#if (DEFAULT_LIGHT_SENSOR_TIMEOUT_SEC < 3*DEFAULT_CYCLE_UPLOAD_STATUS_TIME_SEC)
#error "suggest to config DEFAULT_LIGHT_SENSOR_TIMEOUT_SEC larger than 3 times of DEFAULT_CYCLE_UPLOAD_STATUS_TIME_SEC at least"
#endif

#define DEFAULT_THRESHOLD_INVALID							(9999)


#define UPDATE_LIGHT_SCHEDULE_PERIOD_SEC					30 // secs
#define WAIT_DISCONNECT_TIMEOUT_SEC							(30*60 + 120) // secs
															// NETWORK_TIMEOUT in AT_CellularContext.cpp is 30mins

#define DEBUG_DISPLAY_PERIOD_SEC							5 //secs
#define DEBUG_DISPLAY_PERIOD_LONG_SEC						10 //secs


//-----------------------------------------------------------------------------------//
// feature
//-----------------------------------------------------------------------------------//
#define _TO_CLOSE_NETWORK



//-----------------------------------------------------------------------------------//
// application state of state machine
//-----------------------------------------------------------------------------------//
typedef enum {
	SLCAPP_APPSTATE_RESET,
	SLCAPP_APPSTATE_READY_TO_IDLE,
	SLCAPP_APPSTATE_IDLE,
	SLCAPP_APPSTATE_WAIT_NETWORK_CONNECT,
	SLCAPP_APPSTATE_INIT,
	SLCAPP_APPSTATE_WAIT_PELION_REG,
	SLCAPP_APPSTATE_WAIT_DEV_REG,
	SLCAPP_APPSTATE_WAIT_RESOURCE_SUBSCRIBED,
	SLCAPP_APPSTATE_READY_TO_STANDBY,
	SLCAPP_APPSTATE_STANDBY,
	SLCAPP_APPSTATE_TO_PELION_DEREG,
	SLCAPP_APPSTATE_WAIT_PELION_DEREG,
	SLCAPP_APPSTATE_WAIT_NETWORK_DISCONNECTED
} _SlcApp_AppState;

typedef enum {
	SLCAPP_DEVREGSTATE_INIT,
	SLCAPP_DEVREGSTATE_WAIT_REG,
	SLCAPP_DEVREGSTATE_DONE
} _SlcApp_DevRegState;

//-----------------------------------------------------------------------------------//
// data package format
//-----------------------------------------------------------------------------------//
//----- system state -----//
typedef union {
	struct {
		bool ErrorVoltage : 1;			// bit 0 -> value 1
		bool ErrorCurrent : 1;			// bit 1 -> value 2
		bool ErrorFreq    : 1;			// bit 2 -> value 4
		bool ErrorPf      : 1;			// bit 3 -> value 8
		bool ErrorWatt    : 1;			// bit 4 -> value 16
	} bits;
	uint32_t 						status;
} _SlcApp_Status;

//----- system config -----//
#define SLCAPP_CYCLETIMESTRUCT_LEN		4
#define SLCAPP_DATETIMESTRUCT_LEN		7
#define SLCAPP_DEREGISTERSTRUCT_LEN		2
#define SCLAPP_CONFIGBUFF_LEN			(4 + 4 + 2 + 2 + SLCAPP_CYCLETIMESTRUCT_LEN*5 + SLCAPP_DEREGISTERSTRUCT_LEN + 1 + 1)	// 36

typedef union {
	struct {
		uint16_t					cycle_time	: 15;		// range 0~32767, periodical time in seconds
		bool						random		: 1;		// range 0~1, 1: enable random shift, 0: disable random shift
		uint8_t						diff_time	: 8;		// range 0~255, shift time in seconds
		uint8_t									: 8;
	} bits;
	uint8_t 						buff[SLCAPP_CYCLETIMESTRUCT_LEN];
} _SlcApp_CycleTimeStruct;

typedef union {
	struct {
		bool						keep_online			: 1;	// range 0~1, 0 for deregister pelion and disconnect network after upload status
																//			  1 for always keep online, not to deregister nor disconnect
		_DisconnectType				disconnect_type		: 2;	// range 0~2, 0 for just disconnect network
																//			  1 to send RAI flag after disconnect network
																//			  2 to detach after disconnect network
		uint16_t					keep_online_time	: 13;	// range 0~16383, remain online time in seconds after upload status and then disconnect if keep_online as false
	} bits;
	uint8_t 						buff[SLCAPP_DEREGISTERSTRUCT_LEN];
} _SlcApp_DeregisterStruct;

typedef union {
	struct {
		uint16_t					version;						// config version
		bool						store_node_registered;			// true:  restore "node_registered" from file system when boot-up
																	// false: "node_registered" will be set to false when each boot-up
		uint8_t						reserved;						// reserved for aligment

		uint32_t					sys_watchdog_timeout_sec;		// for system watchdog, trigger watchdog to reset mcu if overtime
																	// range: 0~4294967295 (49710 days)
		uint16_t					reg_watchdog_timeout_sec;		// for waiting node_registered, disconnect and wait for next procedure if overtime
																	// range: 0~65535 (18hr)
		uint16_t					light_sensor_timeout_sec;		// to confirm keeping communication with cloud, for trigger light sensor mode if overtime
																	// range: 0~65535 (18hr)
		_SlcApp_CycleTimeStruct		first_upload_procedure;			// for trigger 1st upload procedure
		_SlcApp_CycleTimeStruct		cycle_upload_procedure;			// for trigger upload procedure periodically after node has registered
		_SlcApp_CycleTimeStruct		cycle_upload_noderegister;		// for keep upload node_registered value to Pelion before device registered
		_SlcApp_CycleTimeStruct		cycle_upload_status;			// for upload ststus periodically
		_SlcApp_CycleTimeStruct		cycle_upload_data;				// for upload data periodically
		
		_SlcApp_DeregisterStruct	deregister;						// for scenario after upload procedure
		bool						cycle_upload_data_enable;		// true: enable upload data periodically
																	// false: disable upload data periodically
		bool						node_registered;				// true: node device has been registered by backend cloud
																	// false: node device has not been registered by backend cloud
	} config;
	uint8_t 						buff[SCLAPP_CONFIGBUFF_LEN];
} _SlcApp_Config;

//----- light dim -----//
#define SLCAPP_LIGHTDIMSTRUCT_LEN		(LIGHT_ADDR_MAX + 1)

typedef union {
	struct {
		uint8_t						broadcast;						// reference: SLCAPP_DIM_ALLDIM, SLCAPP_DIM_BROADCAST, SLCAPP_DIM_SPECIFICDIM
		uint8_t						dimarray[LIGHT_ADDR_MAX];
	} config;
	uint8_t 						buff[SLCAPP_LIGHTDIMSTRUCT_LEN];
} _SlcApp_LightDimStruct;

//----- light schedule -----//
#define SLCAPP_LIGHTSCHEDULESTRUCT_LEN	(SLCAPP_DATETIMESTRUCT_LEN + 1 + 1 + 1)
#define SLCAPP_LIGHTSCHEDULEGROUP_LEN	(SLCAPP_LIGHTSCHEDULESTRUCT_LEN * LIGHT_SCHEDULE_COUNT_MAX)

typedef union {
	struct {
		uint8_t 					Y;	// range 0~255  max 255
		uint8_t 					M;	// range 1~12  max 255
		uint8_t 					D;	// range 1~31  max 255
		uint8_t 					WD; // range 0~6   max 255
		uint8_t 					h;	// range 0~23  max 255
		uint8_t 					m;	// range 0~59  max 255
		uint8_t 					s;	// range 0~59  max 255
	} bits;
	uint8_t 						buff[SLCAPP_DATETIMESTRUCT_LEN];
} _SlcApp_DateTimeStruct;

typedef struct {
	_SlcApp_DateTimeStruct			datetime;		// schedule date & time
													// config full date and time means match for exactlly
													// config full date and time except setting year as 0 for yearly match check
													// config full date and time except setting year and month as 0 for monthly match check
													// config full date and time except setting year and month and day as 0 for weekly match check
													// config full date and time except setting year and month and day and weekday as 0 for daily match check
	uint8_t							enable;			// true: enable light schedule, trigger setting dim value when right on the time
													// false: disable light schedule
	uint8_t							dim;			// dim value
	uint8_t							repeat;			// 0: set dim when time match and disable
													// 1~0xFE: set dim when time match and repeat counter -1
													// 0xFF: always repate
} _SlcApp_LightScheduleStruct;

typedef union {
	_SlcApp_LightScheduleStruct		light_schedule[LIGHT_SCHEDULE_COUNT_MAX];
	uint8_t 						buff[SLCAPP_LIGHTSCHEDULEGROUP_LEN];
} _SlcApp_LightScheduleGroupStruct;


//-----------------------------------------------------------------------------------//
// function declaration
//-----------------------------------------------------------------------------------//
void SlcApp_Upload_NodeRegister(void);
void SlcApp_Upload_Config(void);
void SlcApp_Upload_ThresholdVolatgeHigh(void);
void SlcApp_Upload_ThresholdVolatgeLow(void);
void SlcApp_Upload_ThresholdCurrentHigh(void);
void SlcApp_Upload_ThresholdCurrentLow(void);
void SlcApp_Upload_ThresholdFreqHigh(void);
void SlcApp_Upload_ThresholdFreqLow(void);
void SlcApp_Upload_ThresholdPfHigh(void);
void SlcApp_Upload_ThresholdPfLow(void);
void SlcApp_Upload_ThresholdWattHigh(void);
void SlcApp_Upload_ThresholdWattLow(void);
void SlcApp_Upload_ThresholdLuxHigh(void);
void SlcApp_Upload_ThresholdLuxLow(void);
void SlcApp_Upload_Config_WatchdogSysSec(void);
void SlcApp_Upload_Config_WatchdogRegSec(void);
void SlcApp_Upload_Config_WatchdogLightSensorSec(void);
void SlcApp_Upload_Config_PeriodFirstUploadSec(void);
void SlcApp_Upload_Config_PeriodCycleUploadSec(void);
void SlcApp_Upload_Config_PeriodNodeRegisterSec(void);
void SlcApp_Upload_Config_PeriodStatusSec(void);
void SlcApp_Upload_Config_PeriodDataSec(void); 
void SlcApp_Upload_Config_DeregKeepOnline(void);
void SlcApp_Upload_Config_DeregKeepOnlineTimeSec(void);
void SlcApp_Upload_Status(void);
void SlcApp_Upload_NodeVersion(void);
void SlcApp_Upload_McuVersion(void);
void SlcApp_Upload_Imei(void);
void SlcApp_Upload_Imsi(void);
void SlcApp_Upload_Rssi(void);
void SlcApp_Upload_ModFwVersion(void);
void SlcApp_Upload_CellId(void);
void SlcApp_Upload_Rsrp(void);
void SlcApp_Upload_Rsrq(void);
void SlcApp_Upload_Snr(void);
void SlcApp_Upload_Ccid(void);
void SlcApp_Upload_PowerVol(void);
void SlcApp_Upload_PowerFreq(void);
void SlcApp_Upload_PowerPF(void);
void SlcApp_Upload_PowerCurrent(void);
void SlcApp_Upload_PowerWatt(void);
void SlcApp_Upload_PowerTotalWatt(void);
void SlcApp_Upload_LightDim(void);
void SlcApp_Upload_LightLux(void);
void SlcApp_Upload_McuMode(void);
void SlcApp_Upload_LightSchedule(void);
void SlcApp_Upload_SignalPowerLight(void);
void SlcApp_Upload_CustomAtCmd(void);

void SlcApp_Update_NodeRegister(int64_t value);
void SlcApp_Update_Config(uint8_t *buff, uint32_t size);
void SlcApp_Update_ThresholdVolatgeHigh(float value);
void SlcApp_Update_ThresholdVolatgeLow(float value);
void SlcApp_Update_ThresholdCurrentHigh(float value);
void SlcApp_Update_ThresholdCurrentLow(float value);
void SlcApp_Update_ThresholdFreqHigh(float value);
void SlcApp_Update_ThresholdFreqLow(float value);
void SlcApp_Update_ThresholdPfHigh(float value);
void SlcApp_Update_ThresholdPfLow(float value);
void SlcApp_Update_ThresholdWattHigh(float value);
void SlcApp_Update_ThresholdWattLow(float value);
void SlcApp_Update_ThresholdLuxHigh(int64_t value);
void SlcApp_Update_ThresholdLuxLow(int64_t value);
void SlcApp_Update_Config_WatchdogSysSec(int64_t value);
void SlcApp_Update_Config_WatchdogRegSec(int64_t value);
void SlcApp_Update_Config_WatchdogLightSensorSec(int64_t value);
void SlcApp_Update_Config_PeriodFirstUploadSec(int64_t value);
void SlcApp_Update_Config_PeriodCycleUploadSec(int64_t value);
void SlcApp_Update_Config_PeriodNodeRegisterSec(int64_t value);
void SlcApp_Update_Config_PeriodStatusSec(int64_t value);
void SlcApp_Update_Config_PeriodDataSec(int64_t value);
void SlcApp_Update_Config_DeregKeepOnline(bool value);
void SlcApp_Update_Config_DeregKeepOnlineTimeSec(int64_t value);
void SlcApp_Update_LightDim(uint8_t *buff, int buff_size);
void SlcApp_Update_LightSchedule(uint8_t *buff, uint32_t size);
void SlcApp_Update_CustomAtCmd(char *buff);


void SlcApp_Init(void);
void SlcApp_KickAppTimer(void);
void SlcApp_ResetConfig(void);

void SlcApp_FWUpdateTriggered(void);
void SlcApp_FWUpdateTerminated(void);


#endif // #ifndef _SLC_APPLICATION_H

