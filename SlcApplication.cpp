#include "common_def.h"
#if defined(_PROJECT_SLC) || defined(_PROJECT_SPM)

//#include "mbed_include.h"
#include "mbed.h"
#include <stdlib.h>
#include "SlcApplication.h"
#include "SlcControl.h"
#include "SlcLwm2mData.h"
#include "SlcDataStore.h"
#include "DataStore.h"
#include "CellularExternalControl.h"
#include "led.h"

#include "common_debug.h"
#ifdef _DEBUG_SLCAPP
#define TAG					"SCLAPP"
#define APP_DEBUG			DEBUGMSG
#define APP_DUMP			DEBUGMSG_DUMP
#define APP_ERR				DEBUGMSG_ERR
#else
#define APP_DEBUG
#define APP_DUMP
#define APP_ERR
#endif //_DEBUG_SCLCTRL



bool schedule_debug_show = true;

//-----------------------------------------------------------------------------------//
// local structure declaration
//-----------------------------------------------------------------------------------//
typedef struct {
	bool LaunchUpload;
	bool TerminateUpload;
	bool UploadNodeRegister;
	bool UploadStatus;
	bool UploadData;
	bool UploadDim;
	bool FWUpdate;
	bool FWUpdateTerminated;
	bool UploadCustomAtCmd;
} _SlcApp_Trigger;

typedef struct {
	time_t				UploadTimeStamp;		// time stamp to upload something
	uint8_t				FirstUploadDone;		// first upload procedure has been done after boot-up
	bool				FWUpdating;				// FW update in processing
	uint8_t				McuMode;				// mcu mode, paired mode or sensor mode
	
	bool				TimerTerminateUpload;	// flag for launch timer
	bool				TimerNodeRegister;		// flag for launch timer
	bool				TimerStatus;			// flag for launch timer
	bool				TimerData;				// flag for launch timer
	bool				TimerLightSensor;		// flag for launch timer
	bool				TimerSystemWatchdog;	// flag for launch timer
	bool				TimerThreadWatchdog;	// flag for launch timer

	_SlcApp_Trigger		Trigger;				// flag to trigger all kinds of action

	char				CustomAtCmd[SLC_RESOURCE_MAX_AT_CMD_STRING_SIZE];
	bool				CustomAtCmdTooLong;
} _SlcApp;

typedef struct {
	float VoltageHigh;
	float VoltageLow;
	float CurrentHigh;
	float CurrentLow;
	float FreqHigh;
	float FreqLow;
	float PfHigh;
	float PfLow;
	float WattHigh;
	float WattLow;
	uint32_t LuxHigh;
	uint32_t LuxLow;
} _SlcApp_Threshold;


//-----------------------------------------------------------------------------------//
// function declaration
//-----------------------------------------------------------------------------------//
static void SlcApp_TimerLaunchUploadLaunch(bool random, uint16_t interval, uint8_t random_diff);
static void SlcApp_TimerLaunchUploadFxn(void const *n);
static void SlcApp_TimerTerminateUploadFxn(void const *n);
static void SlcApp_TimerNodeRegisterLaunch(bool random, uint16_t interval, uint8_t random_diff);
static void SlcApp_TimerNodeRegisterFxn(void const *n);
static void SlcApp_TimerStatusLaunch(bool random, uint16_t interval, uint8_t random_diff);
static void SlcApp_TimerStatusFxn(void const *n);
static void SlcApp_TimerDataLaunch(bool random, uint16_t interval, uint8_t random_diff);
static void SlcApp_TimerDataFxn(void const *n);
static void SlcApp_TimerLightSensorModeFxn(void const *n);
static void SlcApp_TimerSystemWatchdogFxn(void const *n);
static void SlcApp_TimerThreadWatchdogFxn(void const *n);
static void SlcApp_KickThreadTimer(void);

static bool SlcApp_CheckOutOfSpec(float value, float th_h, float th_l);


//-----------------------------------------------------------------------------------//
// global variables
//-----------------------------------------------------------------------------------//
static _SlcApp							SlcApp;
static _SlcApp_Status					SlcAppStatus;
static _SlcApp_Config					SlcAppConfig;
static _SlcApp_Config					SlcAppConfigBackup;
static _SlcApp_LightDimStruct 			SlcAppLightDim;
static _SlcApp_LightDimStruct 			SlcAppLightDimBackup;
static _SlcApp_LightScheduleGroupStruct	SlcAppLightScheduleGroup;
static _SlcApp_Threshold				SlcAppTh;

#ifdef _DEBUG_SLCAPP
#define SLCAPP_STACKSIZE	2560
#else
#define SLCAPP_STACKSIZE	1024
#endif
static uint8_t SlcAppStack[SLCAPP_STACKSIZE];
//Thread SlcApp_Thread(osPriorityNormal, 2560, NULL, "slcapp_thread"); //(osPriorityNormal, MBED_CONF_APP_SLCCTRL_STACK_SIZE, _slcctrl_stack, "slcapp_thread");
Thread SlcApp_Thread(osPriorityNormal, SLCAPP_STACKSIZE, SlcAppStack, "slcapp_thread");

RtosTimer SlcApp_TimerLaunchUpload(SlcApp_TimerLaunchUploadFxn, osTimerOnce, (void *)0);
RtosTimer SlcApp_TimerTerminateUpload(SlcApp_TimerTerminateUploadFxn, osTimerOnce, (void *)0);
RtosTimer SlcApp_TimerNodeRegister(SlcApp_TimerNodeRegisterFxn, osTimerOnce, (void *)0);
RtosTimer SlcApp_TimerStatus(SlcApp_TimerStatusFxn, osTimerOnce, (void *)0);
RtosTimer SlcApp_TimerData(SlcApp_TimerDataFxn, osTimerOnce, (void *)0);
RtosTimer SlcApp_TimerLightSensorMode(SlcApp_TimerLightSensorModeFxn, osTimerPeriodic, (void *)0);
RtosTimer SlcApp_TimerSystemWatchdog(SlcApp_TimerSystemWatchdogFxn, osTimerPeriodic, (void *)0);
RtosTimer SlcApp_TimerThreadWatchdog(SlcApp_TimerThreadWatchdogFxn, osTimerPeriodic, (void *)0);


//-----------------------------------------------------------------------------------//
// macro declaration
//-----------------------------------------------------------------------------------//
// timer - launch 1st power-on upload
#define SLCAPP_LAUNCHTIMER_1ST_LAUNCHUPLOAD()		SlcApp_TimerLaunchUploadLaunch(true, DEFAULT_POWERON_FIRST_UPLOAD_PROCEDURE_TIME_SEC, DEFAULT_POWERON_FIRST_UPLOAD_PROCEDURE_RANDOM_RANGE_SEC)

// timer - launch upload with reduced period
#define SLCAPP_LAUNCHTIMER_REDUCE_PERIOD_LAUNCHUPLOAD()	SlcApp_TimerLaunchUploadLaunch(SlcAppConfig.config.first_upload_procedure.bits.random, SlcAppConfig.config.first_upload_procedure.bits.cycle_time, SlcAppConfig.config.first_upload_procedure.bits.diff_time)

// timer - launch upload
#define SLCAPP_LAUNCHTIMER_LAUNCHUPLOAD()			SlcApp_TimerLaunchUploadLaunch(SlcAppConfig.config.cycle_upload_procedure.bits.random, SlcAppConfig.config.cycle_upload_procedure.bits.cycle_time, SlcAppConfig.config.cycle_upload_procedure.bits.diff_time)

// timer - register watchdog
#define SLCAPP_STOPTIMER_REG_WATCHDOG()			do { \
													if(SlcApp.TimerTerminateUpload) SlcApp_TimerTerminateUpload.stop(); \
													SlcApp.Trigger.TerminateUpload = false; \
													SlcApp.TimerTerminateUpload = false; \
												} while(0)
#define SLCAPP_RELAUNCHTIMER_REG_WATCHDOG()		do { \
													if(SlcApp.TimerTerminateUpload) SlcApp_TimerTerminateUpload.stop(); \
													SlcApp_TimerTerminateUpload.start(SlcAppConfig.config.reg_watchdog_timeout_sec*1000); \
													SlcApp.TimerTerminateUpload = true; \
												} while(0)												

#define SLCAPP_RELAUNCHTIMER_NODEREG_WATCHDOG()	do { \
												if(SlcApp.TimerTerminateUpload) SlcApp_TimerTerminateUpload.stop(); \
													SlcApp_TimerTerminateUpload.start(DEFAULT_NODEREG_WATCHDOG_TIMEOUT_SEC*1000); \
													SlcApp.TimerTerminateUpload = true; \
												} while(0)

// timer - terminate upload
#define SLCAPP_STOPTIMER_TERMINATEUPLOAD()		do { \
													if(SlcApp.TimerTerminateUpload) SlcApp_TimerTerminateUpload.stop(); \
													SlcApp.Trigger.TerminateUpload = false; \
													SlcApp.TimerTerminateUpload = false; \
												} while(0)
#define SLCAPP_RELAUNCHTIMER_TERMINATEUPLOAD()	do { \
													if(SlcApp.TimerTerminateUpload) SlcApp_TimerTerminateUpload.stop(); \
													SlcApp_TimerTerminateUpload.start(SlcAppConfig.config.deregister.bits.keep_online_time*1000); \
													SlcApp.TimerTerminateUpload = true; \
												} while(0)


// timer - upload node register
#define SLCAPP_STOPTIMER_NODEREGISTER()			do { \
													if(SlcApp.TimerNodeRegister) SlcApp_TimerNodeRegister.stop(); \
													SlcApp.Trigger.UploadNodeRegister = false; \
													SlcApp.TimerNodeRegister = false; \
												} while(0)
#define SLCAPP_LAUNCHTIMER_NODEREGISTER()		do { \
													SlcApp_TimerNodeRegisterLaunch(SlcAppConfig.config.cycle_upload_noderegister.bits.random, SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time, SlcAppConfig.config.cycle_upload_noderegister.bits.diff_time); \
													SlcApp.TimerNodeRegister = true; \
												} while(0)

// timer - upload status
#define SLCAPP_STOPTIMER_STATUS()				do { \
													if(SlcApp.TimerStatus) SlcApp_TimerStatus.stop(); \
													SlcApp.Trigger.UploadStatus = false; \
													SlcApp.TimerStatus = false; \
												} while(0)
#define SLCAPP_LAUNCHTIMER_STATUS()				do { \
													SlcApp_TimerStatusLaunch(SlcAppConfig.config.cycle_upload_status.bits.random, SlcAppConfig.config.cycle_upload_status.bits.cycle_time, SlcAppConfig.config.cycle_upload_status.bits.diff_time); \
													SlcApp.TimerStatus = true; \
												} while(0)

// timer - upload data
#define SLCAPP_STOPTIMER_DATA()					do { \
													if(SlcApp.TimerData) SlcApp_TimerData.stop(); \
													SlcApp.Trigger.UploadData = false; \
													SlcApp.TimerData = false; \
												} while(0)
#define SLCAPP_RELAUNCHTIMER_DATA()				do { \
													if(SlcApp.TimerData) SlcApp_TimerData.stop(); \
													SlcApp_TimerDataLaunch(SlcAppConfig.config.cycle_upload_data.bits.random, SlcAppConfig.config.cycle_upload_data.bits.cycle_time, SlcAppConfig.config.cycle_upload_data.bits.diff_time); \
													SlcApp.TimerData = true; \
												} while(0)
												
// timer - light sensor mode
#define SLCAPP_RELAUNCHTIMER_LIGHTSENSORMODE()	do { \
													if(SlcApp.TimerLightSensor) SlcApp_TimerLightSensorMode.stop(); \
													SlcApp_TimerLightSensorMode.start(SlcAppConfig.config.light_sensor_timeout_sec*1000); \
													SlcApp.TimerLightSensor = true; \
												} while(0)

// timer - system watchdog
#define SLCAPP_RELAUNCHTIMER_SYSTEMWATCHDOG()	do { \
													if(SlcApp.TimerSystemWatchdog) SlcApp_TimerSystemWatchdog.stop(); \
													SlcApp_TimerSystemWatchdog.start(SlcAppConfig.config.sys_watchdog_timeout_sec*1000); \
													SlcApp.TimerSystemWatchdog = true; \
												} while(0)
													
// timer - thread watchdog
#define SLCAPP_RELAUNCHTIMER_THREADWATCHDOG()	do { \
													if(SlcApp.TimerThreadWatchdog) SlcApp_TimerThreadWatchdog.stop(); \
													SlcApp_TimerThreadWatchdog.start(THREAD_WATCHDOG_TIMEOUTTIME_SEC*1000); \
													SlcApp.TimerThreadWatchdog = true; \
												} while(0)


// check threshold
#define SLCAPP_CHECKOUTOFSPEC_VOLTAGE() SlcApp_CheckOutOfSpec(SlcCtrl_GetVoltage(), SlcAppTh.VoltageHigh, SlcAppTh.VoltageLow)
#define SLCAPP_CHECKOUTOFSPEC_CURRENT() SlcApp_CheckOutOfSpec(SlcCtrl_GetCurrent(), SlcAppTh.CurrentHigh, SlcAppTh.CurrentLow)
#define SLCAPP_CHECKOUTOFSPEC_FREQ()	SlcApp_CheckOutOfSpec(SlcCtrl_GetFreq(),	SlcAppTh.FreqHigh,	  SlcAppTh.FreqLow)
#define SLCAPP_CHECKOUTOFSPEC_PF()		SlcApp_CheckOutOfSpec(SlcCtrl_GetPF(),		SlcAppTh.PfHigh,	  SlcAppTh.PfLow)
#define SLCAPP_CHECKOUTOFSPEC_WATT()	SlcApp_CheckOutOfSpec(SlcCtrl_GetWatt(),	SlcAppTh.WattHigh,	  SlcAppTh.WattLow)


// led
#define SLCAPP_LED_IDLE()						do { \
													LED_RED_BLINK_SLOW(); \
													LED_GREEN_OFF(); \
													LED_GREEN_OFF(); \
												} while(0)
#define SLCAPP_LED_NETWORK_CONNECTING()			do { \
													LED_RED_BLINK_NORMAL(); \
													LED_GREEN_OFF(); \
													LED_GREEN_OFF(); \
												} while(0)
#define SLCAPP_LED_PELION_REGISTERING()			do { \
													LED_RED_OFF(); \
													LED_RED_OFF(); \
													LED_GREEN_BLINK_NORMAL(); \
												} while(0)
//#define SLCAPP_LED_DEVICE_REGISTERING()			do { \
//													LED_RED_OFF(); \
//													LED_RED_OFF(); \
//													LED_GREEN_BLINK_SLOW(); \
//												} while(0)
#define SLCAPP_LED_RESOURCE_SUBSCRIBING()		do { \
													LED_RED_OFF(); \
													LED_RED_OFF(); \
													LED_GREEN_BLINK_SLOW(); \
												} while(0)
#define SLCAPP_LED_UPLOAD_STRANDBY()			do { \
													LED_RED_OFF(); \
													LED_RED_OFF(); \
													LED_GREEN_ON(); \
												} while(0)
#define SLCAPP_LED_DISCONNECTING()				do { \
													LED_RED_OFF(); \
													LED_RED_OFF(); \
													LED_GREEN_BLINK_FAST(); \
												} while(0)


//------------------------------------------------------------------------------------------//
// update data interface functions
//------------------------------------------------------------------------------------------//
void SlcApp_Update_NodeRegister(int64_t value)
{
	SlcAppConfigBackup.config.node_registered = SlcAppConfig.config.node_registered;
	SlcAppConfig.config.node_registered = (value > 0)? true : false;
	
	APP_DEBUG("node_registered : %d -> %d", SlcAppConfigBackup.config.node_registered, SlcAppConfig.config.node_registered);

	if(SlcAppConfig.config.node_registered != SlcAppConfigBackup.config.node_registered)
	{
		DataStore_WriteFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, SlcAppConfig.buff, sizeof(SlcAppConfig.buff), false);
		APP_DEBUG("config updated and changed, store");
	}
}
void SlcApp_Update_Config(uint8_t *buff, uint32_t size)
{
	if(size == sizeof(SlcAppConfig.buff))
	{
		memcpy(SlcAppConfigBackup.buff, SlcAppConfig.buff, size);
		memcpy(SlcAppConfig.buff, buff, size);

		APP_DEBUG("config version                        : %d -> %d", SlcAppConfigBackup.config.version, SlcAppConfig.config.version);
		APP_DEBUG("store_node_registered                 : %d -> %d", SlcAppConfigBackup.config.store_node_registered, SlcAppConfig.config.store_node_registered);

		APP_DEBUG("sys_watchdog_timeout_sec              : %d -> %d", SlcAppConfigBackup.config.sys_watchdog_timeout_sec, SlcAppConfig.config.sys_watchdog_timeout_sec);
		APP_DEBUG("reg_watchdog_timeout_sec              : %d -> %d", SlcAppConfigBackup.config.reg_watchdog_timeout_sec, SlcAppConfig.config.reg_watchdog_timeout_sec);
		APP_DEBUG("light_sensor_timeout_sec              : %d -> %d", SlcAppConfigBackup.config.light_sensor_timeout_sec, SlcAppConfig.config.light_sensor_timeout_sec);

		APP_DEBUG("first_upload_procedure - random       : %d -> %d", SlcAppConfigBackup.config.first_upload_procedure.bits.random, SlcAppConfig.config.first_upload_procedure.bits.random);
		APP_DEBUG("first_upload_procedure - cycle_time   : %d -> %d", SlcAppConfigBackup.config.first_upload_procedure.bits.cycle_time, SlcAppConfig.config.first_upload_procedure.bits.cycle_time);
		APP_DEBUG("first_upload_procedure - diff_time    : %d -> %d", SlcAppConfigBackup.config.first_upload_procedure.bits.diff_time, SlcAppConfig.config.first_upload_procedure.bits.diff_time);
		
		APP_DEBUG("cycle_upload_procedure - random       : %d -> %d", SlcAppConfigBackup.config.cycle_upload_procedure.bits.random, SlcAppConfig.config.cycle_upload_procedure.bits.random);
		APP_DEBUG("cycle_upload_procedure - cycle_time   : %d -> %d", SlcAppConfigBackup.config.cycle_upload_procedure.bits.cycle_time, SlcAppConfig.config.cycle_upload_procedure.bits.cycle_time);
		APP_DEBUG("cycle_upload_procedure - diff_time    : %d -> %d", SlcAppConfigBackup.config.cycle_upload_procedure.bits.diff_time, SlcAppConfig.config.cycle_upload_procedure.bits.diff_time);
		
		APP_DEBUG("cycle_upload_noderegister - random    : %d -> %d", SlcAppConfigBackup.config.cycle_upload_noderegister.bits.random, SlcAppConfig.config.cycle_upload_noderegister.bits.random);			
		APP_DEBUG("cycle_upload_noderegister - cycle_time: %d -> %d", SlcAppConfigBackup.config.cycle_upload_noderegister.bits.cycle_time, SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time);
		APP_DEBUG("cycle_upload_noderegister - diff_time : %d -> %d", SlcAppConfigBackup.config.cycle_upload_noderegister.bits.diff_time, SlcAppConfig.config.cycle_upload_noderegister.bits.diff_time);
		
		APP_DEBUG("cycle_upload_status - random          : %d -> %d", SlcAppConfigBackup.config.cycle_upload_status.bits.random, SlcAppConfig.config.cycle_upload_status.bits.random);
		APP_DEBUG("cycle_upload_status - cycle_time      : %d -> %d", SlcAppConfigBackup.config.cycle_upload_status.bits.cycle_time, SlcAppConfig.config.cycle_upload_status.bits.cycle_time);
		APP_DEBUG("cycle_upload_status - diff_time       : %d -> %d", SlcAppConfigBackup.config.cycle_upload_status.bits.diff_time, SlcAppConfig.config.cycle_upload_status.bits.diff_time);

		APP_DEBUG("cycle_upload_data - random            : %d -> %d", SlcAppConfigBackup.config.cycle_upload_data.bits.random, SlcAppConfig.config.cycle_upload_data.bits.random);
		APP_DEBUG("cycle_upload_data - cycle_time        : %d -> %d", SlcAppConfigBackup.config.cycle_upload_data.bits.cycle_time, SlcAppConfig.config.cycle_upload_data.bits.cycle_time);
		APP_DEBUG("cycle_upload_data - diff_time         : %d -> %d", SlcAppConfigBackup.config.cycle_upload_data.bits.diff_time, SlcAppConfig.config.cycle_upload_data.bits.diff_time);

		APP_DEBUG("deregister - keep_online              : %d -> %d", SlcAppConfigBackup.config.deregister.bits.keep_online, SlcAppConfig.config.deregister.bits.keep_online);
		APP_DEBUG("deregister - disconnect_type          : %d -> %d", SlcAppConfigBackup.config.deregister.bits.disconnect_type, SlcAppConfig.config.deregister.bits.disconnect_type);
		APP_DEBUG("deregister - keep_online_time         : %d -> %d", SlcAppConfigBackup.config.deregister.bits.keep_online_time, SlcAppConfig.config.deregister.bits.keep_online_time);
		
		APP_DEBUG("cycle_upload_data_enable              : %d -> %d", SlcAppConfigBackup.config.cycle_upload_data_enable, SlcAppConfig.config.cycle_upload_data_enable);		
		APP_DEBUG("node_registered                       : %d -> %d", SlcAppConfigBackup.config.node_registered, SlcAppConfig.config.node_registered);

		if(SlcAppConfig.config.first_upload_procedure.bits.cycle_time < CYCLE_UPLOAD_TIME_MIN_SEC)
			SlcAppConfig.config.first_upload_procedure.bits.cycle_time = CYCLE_UPLOAD_TIME_MIN_SEC;
		if(SlcAppConfig.config.cycle_upload_procedure.bits.cycle_time < CYCLE_UPLOAD_TIME_MIN_SEC)
			SlcAppConfig.config.cycle_upload_procedure.bits.cycle_time = CYCLE_UPLOAD_TIME_MIN_SEC;
		if(SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time < CYCLE_UPLOAD_TIME_MIN_SEC)
			SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time = CYCLE_UPLOAD_TIME_MIN_SEC;
		if(SlcAppConfig.config.cycle_upload_status.bits.cycle_time < CYCLE_UPLOAD_TIME_MIN_SEC)
			SlcAppConfig.config.cycle_upload_status.bits.cycle_time = CYCLE_UPLOAD_TIME_MIN_SEC;
		if(SlcAppConfig.config.cycle_upload_data.bits.cycle_time < CYCLE_UPLOAD_TIME_MIN_SEC)
			SlcAppConfig.config.cycle_upload_data.bits.cycle_time = CYCLE_UPLOAD_TIME_MIN_SEC;

		if(!memcmp(SlcAppConfig.buff, SlcAppConfigBackup.buff, sizeof(SlcAppConfig.buff)))
		{
			DataStore_WriteFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, SlcAppConfig.buff, sizeof(SlcAppConfig.buff), false);
			APP_DEBUG("config updated and changed, store");
		}
	}
	else
		APP_ERR("[SlcApp] config buff size not match !!!\n");
}
void SlcApp_Update_ThresholdVolatgeHigh(float value)
{
	if(value != SlcAppTh.VoltageHigh)
	{
		APP_DEBUG("to update VoltageHigh: %f -> %f", SlcAppTh.VoltageHigh, value);
		SlcAppTh.VoltageHigh = value;
		DataStore_WriteFile(SLCDATASTORE_VOLTAGE_THRESHOLD_H, (uint8_t *)&SlcAppTh.VoltageHigh, sizeof(SlcAppTh.VoltageHigh), false);		
	}
}
void SlcApp_Update_ThresholdVolatgeLow(float value)
{
	if(value != SlcAppTh.VoltageLow)
	{
		APP_DEBUG("to update VoltageLow: %f -> %f", SlcAppTh.VoltageLow, value);
		SlcAppTh.VoltageLow = value;
		DataStore_WriteFile(SLCDATASTORE_VOLTAGE_THRESHOLD_L, (uint8_t *)&SlcAppTh.VoltageLow, sizeof(SlcAppTh.VoltageLow), false);		
	}
}
void SlcApp_Update_ThresholdCurrentHigh(float value)
{
	if(value != SlcAppTh.CurrentHigh)
	{
		APP_DEBUG("to update CurrentHigh: %f -> %f", SlcAppTh.CurrentHigh, value);
		SlcAppTh.CurrentHigh = value;
		DataStore_WriteFile(SLCDATASTORE_CURRENT_THRESHOLD_H, (uint8_t *)&SlcAppTh.CurrentHigh, sizeof(SlcAppTh.CurrentHigh), false);		
	}
}
void SlcApp_Update_ThresholdCurrentLow(float value)
{
	if(value != SlcAppTh.CurrentLow)
	{
		APP_DEBUG("to update CurrentLow: %f -> %f", SlcAppTh.CurrentLow, value);
		SlcAppTh.CurrentLow = value;
		DataStore_WriteFile(SLCDATASTORE_CURRENT_THRESHOLD_L, (uint8_t *)&SlcAppTh.CurrentLow, sizeof(SlcAppTh.CurrentLow), false);		
	}
}
void SlcApp_Update_ThresholdFreqHigh(float value)
{
	if(value != SlcAppTh.FreqHigh)
	{
		APP_DEBUG("to update FreqHigh: %f -> %f", SlcAppTh.FreqHigh, value);
		SlcAppTh.FreqHigh = value;
		DataStore_WriteFile(SLCDATASTORE_FREQ_THRESHOLD_H, (uint8_t *)&SlcAppTh.FreqHigh, sizeof(SlcAppTh.FreqHigh), false);		
	}
}
void SlcApp_Update_ThresholdFreqLow(float value)
{
	if(value != SlcAppTh.FreqLow)
	{
		APP_DEBUG("to update FreqLow: %f -> %f", SlcAppTh.FreqLow, value);
		SlcAppTh.FreqLow = value;
		DataStore_WriteFile(SLCDATASTORE_FREQ_THRESHOLD_L, (uint8_t *)&SlcAppTh.FreqLow, sizeof(SlcAppTh.FreqLow), false);		
	}
}
void SlcApp_Update_ThresholdPfHigh(float value)
{
	if(value != SlcAppTh.PfHigh)
	{
		APP_DEBUG("to update PfHigh: %f -> %f", SlcAppTh.PfHigh, value);
		SlcAppTh.PfHigh = value;
		DataStore_WriteFile(SLCDATASTORE_PF_THRESHOLD_L, (uint8_t *)&SlcAppTh.PfHigh, sizeof(SlcAppTh.PfHigh), false);		
	}
}
void SlcApp_Update_ThresholdPfLow(float value)
{
	if(value != SlcAppTh.PfLow)
	{
		APP_DEBUG("to update PfLow: %f -> %f", SlcAppTh.PfLow, value);
		SlcAppTh.PfLow = value;
		DataStore_WriteFile(SLCDATASTORE_PF_THRESHOLD_H, (uint8_t *)&SlcAppTh.PfLow, sizeof(SlcAppTh.PfLow), false);		
	}
}
void SlcApp_Update_ThresholdWattHigh(float value)
{
	if(value != SlcAppTh.WattHigh)
	{
		APP_DEBUG("to update WattHigh: %f -> %f", SlcAppTh.WattHigh, value);
		SlcAppTh.WattHigh = value;
		DataStore_WriteFile(SLCDATASTORE_WATT_THRESHOLD_H, (uint8_t *)&SlcAppTh.WattHigh, sizeof(SlcAppTh.WattHigh), false);		
	}
}
void SlcApp_Update_ThresholdWattLow(float value)
{
	if(value != SlcAppTh.WattLow)
	{
		APP_DEBUG("to update WattLow: %f -> %f", SlcAppTh.WattLow, value);
		SlcAppTh.WattLow = value;
		DataStore_WriteFile(SLCDATASTORE_WATT_THRESHOLD_L, (uint8_t *)&SlcAppTh.WattLow, sizeof(SlcAppTh.WattLow), false);		
	}
}
void SlcApp_Update_ThresholdLuxHigh(int64_t value)
{
	if(value != SlcAppTh.LuxHigh)
	{
		APP_DEBUG("to update LuxHigh: %f -> %f", SlcAppTh.LuxHigh, value);
		SlcAppTh.LuxHigh = value;
		DataStore_WriteFile(SLCDATASTORE_LUX_THRESHOLD_H, (uint8_t *)&SlcAppTh.LuxHigh, sizeof(SlcAppTh.LuxHigh), false);		
	}
}
void SlcApp_Update_ThresholdLuxLow(int64_t value)
{
	if(value != SlcAppTh.LuxLow)
	{
		APP_DEBUG("to update LuxLow: %f -> %f", SlcAppTh.LuxLow, value);
		SlcAppTh.LuxLow = value;
		DataStore_WriteFile(SLCDATASTORE_LUX_THRESHOLD_L, (uint8_t *)&SlcAppTh.LuxLow, sizeof(SlcAppTh.LuxLow), false);		
	}
}
void SlcApp_Update_Config_WatchdogSysSec(int64_t value)
{
	if((uint32_t)value != SlcAppConfig.config.sys_watchdog_timeout_sec)
	{
		APP_DEBUG("to update sys_watchdog_timeout_sec: %d -> %d", SlcAppConfig.config.sys_watchdog_timeout_sec, value);
		SlcAppConfig.config.sys_watchdog_timeout_sec = (uint32_t)value;
		DataStore_WriteFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, SlcAppConfig.buff, sizeof(SlcAppConfig.buff), false);
	}
}
void SlcApp_Update_Config_WatchdogRegSec(int64_t value)
{
	if((uint32_t)value != SlcAppConfig.config.reg_watchdog_timeout_sec)
	{
		APP_DEBUG("to update reg_watchdog_timeout_sec: %d -> %d", SlcAppConfig.config.reg_watchdog_timeout_sec, value);
		SlcAppConfig.config.reg_watchdog_timeout_sec = (uint32_t)value;
		DataStore_WriteFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, SlcAppConfig.buff, sizeof(SlcAppConfig.buff), false);
	}
}
void SlcApp_Update_Config_WatchdogLightSensorSec(int64_t value)
{
	if((uint32_t)value != SlcAppConfig.config.light_sensor_timeout_sec)
	{
		APP_DEBUG("to update light_sensor_timeout_sec: %d -> %d", SlcAppConfig.config.light_sensor_timeout_sec, value);
		SlcAppConfig.config.light_sensor_timeout_sec = (uint32_t)value;
		DataStore_WriteFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, SlcAppConfig.buff, sizeof(SlcAppConfig.buff), false);
	}
}
void SlcApp_Update_Config_PeriodFirstUploadSec(int64_t value)
{
	if(SlcAppConfig.config.first_upload_procedure.bits.cycle_time < CYCLE_UPLOAD_TIME_MIN_SEC)
	{
		APP_DEBUG("limit first_upload_procedure to %s secs at least", CYCLE_UPLOAD_TIME_MIN_SEC);
		SlcAppConfig.config.first_upload_procedure.bits.cycle_time = CYCLE_UPLOAD_TIME_MIN_SEC;
	}
	if((uint32_t)value != SlcAppConfig.config.first_upload_procedure.bits.cycle_time)
	{
		APP_DEBUG("to update first_upload_procedure: %d -> %d", SlcAppConfig.config.first_upload_procedure.bits.cycle_time, value);
		SlcAppConfig.config.first_upload_procedure.bits.cycle_time = (uint32_t)value;
		DataStore_WriteFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, SlcAppConfig.buff, sizeof(SlcAppConfig.buff), false);
	}
}
void SlcApp_Update_Config_PeriodCycleUploadSec(int64_t value)
{
	if(SlcAppConfig.config.cycle_upload_procedure.bits.cycle_time < CYCLE_UPLOAD_TIME_MIN_SEC)
	{
		APP_DEBUG("limit cycle_upload_procedure cycle time to %s secs at least", CYCLE_UPLOAD_TIME_MIN_SEC);
		SlcAppConfig.config.cycle_upload_procedure.bits.cycle_time = CYCLE_UPLOAD_TIME_MIN_SEC;
	}
	if((uint32_t)value != SlcAppConfig.config.cycle_upload_procedure.bits.cycle_time)
	{
		APP_DEBUG("to update cycle_upload_procedure cycle time: %d -> %d", SlcAppConfig.config.cycle_upload_procedure.bits.cycle_time, value);
		SlcAppConfig.config.cycle_upload_procedure.bits.cycle_time = (uint32_t)value;
		DataStore_WriteFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, SlcAppConfig.buff, sizeof(SlcAppConfig.buff), false);
	}
}
void SlcApp_Update_Config_PeriodNodeRegisterSec(int64_t value)
{
	if(SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time < CYCLE_UPLOAD_TIME_MIN_SEC)
	{
		APP_DEBUG("limit cycle_upload_noderegister cycle time to %s secs at least", CYCLE_UPLOAD_TIME_MIN_SEC);
		SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time = CYCLE_UPLOAD_TIME_MIN_SEC;
	}
	if((uint32_t)value != SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time)
	{
		APP_DEBUG("to update cycle_upload_noderegister cycle time: %d -> %d", SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time, value);
		SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time = (uint32_t)value;
		DataStore_WriteFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, SlcAppConfig.buff, sizeof(SlcAppConfig.buff), false);
	}
}
void SlcApp_Update_Config_PeriodStatusSec(int64_t value)
{
	if(SlcAppConfig.config.cycle_upload_status.bits.cycle_time < CYCLE_UPLOAD_TIME_MIN_SEC)
	{
		APP_DEBUG("limit cycle_upload_status cycle time to %s secs at least", CYCLE_UPLOAD_TIME_MIN_SEC);
		SlcAppConfig.config.cycle_upload_status.bits.cycle_time = CYCLE_UPLOAD_TIME_MIN_SEC;
	}
	if((uint32_t)value != SlcAppConfig.config.cycle_upload_status.bits.cycle_time)
	{
		APP_DEBUG("to update cycle_upload_status cycle time: %d -> %d", SlcAppConfig.config.cycle_upload_status.bits.cycle_time, value);
		SlcAppConfig.config.cycle_upload_status.bits.cycle_time = (uint32_t)value;
		DataStore_WriteFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, SlcAppConfig.buff, sizeof(SlcAppConfig.buff), false);
	}
}
void SlcApp_Update_Config_PeriodDataSec(int64_t value)
{
	if(SlcAppConfig.config.cycle_upload_data.bits.cycle_time < CYCLE_UPLOAD_TIME_MIN_SEC)
	{
		APP_DEBUG("limit cycle_upload_data cycle time to %s secs at least", CYCLE_UPLOAD_TIME_MIN_SEC);
		SlcAppConfig.config.cycle_upload_data.bits.cycle_time = CYCLE_UPLOAD_TIME_MIN_SEC;
	}
	if((uint32_t)value != SlcAppConfig.config.cycle_upload_data.bits.cycle_time)
	{
		APP_DEBUG("to update cycle_upload_data cycle time: %d -> %d", SlcAppConfig.config.cycle_upload_data.bits.cycle_time, value);
		SlcAppConfig.config.cycle_upload_data.bits.cycle_time = (uint32_t)value;
		DataStore_WriteFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, SlcAppConfig.buff, sizeof(SlcAppConfig.buff), false);
	}
}
void SlcApp_Update_Config_DeregKeepOnline(bool value)
{
	if((uint32_t)value != SlcAppConfig.config.deregister.bits.keep_online)
	{
		APP_DEBUG("to update deregister keep_online: %d -> %d", SlcAppConfig.config.deregister.bits.keep_online, value);
		SlcAppConfig.config.deregister.bits.keep_online = value;
		DataStore_WriteFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, SlcAppConfig.buff, sizeof(SlcAppConfig.buff), false);
	}
}
void SlcApp_Update_Config_DeregKeepOnlineTimeSec(int64_t value)
{
	if((uint32_t)value != SlcAppConfig.config.deregister.bits.keep_online_time)
	{
		APP_DEBUG("to update deregister keep_online_time: %d -> %d", SlcAppConfig.config.deregister.bits.keep_online_time, value);
		SlcAppConfig.config.deregister.bits.keep_online_time = (uint32_t)value;
		DataStore_WriteFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, SlcAppConfig.buff, sizeof(SlcAppConfig.buff), false);
	}
}
void SlcApp_Update_LightDim(uint8_t *buff, int buff_size)
{
	_SlcApp_LightDimStruct dimstruct;
	memcpy(dimstruct.buff, buff, buff_size);
	if(dimstruct.config.broadcast == SLCAPP_DIM_SPECIFICDIM)
	{
		int dimcnt;
		memcpy(SlcAppLightDimBackup.buff, SlcAppLightDim.buff, sizeof(SlcAppLightDim.buff));
		SlcAppLightDim.config.broadcast = 0;
		dimcnt = (buff_size-1)/2;
		int i, j, idx;
		j = 1;
		for(i=0; i<dimcnt; i++)
		{
			idx = buff[j++];
			SlcAppLightDim.config.dimarray[idx] = buff[j++];
		}
	}
	else
	{
		memcpy(SlcAppLightDimBackup.buff, SlcAppLightDim.buff, sizeof(SlcAppLightDim.buff));
		buff_size = (buff_size<sizeof(SlcAppLightDim.buff))? buff_size : sizeof(SlcAppLightDim.buff);
		memcpy(SlcAppLightDim.buff, buff, buff_size);
	}
}

void SlcApp_Update_LightSchedule(uint8_t *buff, uint32_t size)
{
	int i;
	if(size == sizeof(SlcAppLightScheduleGroup.buff))
	{
		memcpy(SlcAppLightScheduleGroup.buff, buff, size);

		for(i=0; i<LIGHT_SCHEDULE_COUNT_MAX; i++)
		{
			APP_DEBUG("LighSchedule[%d] enable %d", i, SlcAppLightScheduleGroup.light_schedule[i].enable);
			APP_DEBUG("LighSchedule[%d] repeat %d", i, SlcAppLightScheduleGroup.light_schedule[i].repeat);
			APP_DEBUG("LighSchedule[%d] time %d-%d-%d(%d) %d:%d:%d", i, 
										SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.Y,
										SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.M,
										SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.D,
										SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.WD,
										SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.h,
										SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.m,
										SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.s);
			APP_DEBUG("LighSchedule[%d] dim %d", i, SlcAppLightScheduleGroup.light_schedule[i].dim);
		}

		DataStore_WriteFile(SLCDATASTORE_LIGHTSCHEDULE_FILE_NAME, SlcAppLightScheduleGroup.buff, sizeof(SlcAppLightScheduleGroup.buff), false);
		APP_DEBUG("light schedule updated, store");
	}
	else
		APP_ERR("[SlcApp] light schedule buff size not match !!!\n");
}
void SlcApp_Update_CustomAtCmd(char *buff)
{
	if(strlen(buff) > sizeof(SlcApp.CustomAtCmd))
	{
		SlcApp.CustomAtCmdTooLong = true;
		SlcApp.Trigger.UploadCustomAtCmd = true;
	}
	else
	{
		memset(SlcApp.CustomAtCmd, 0, sizeof(SlcApp.CustomAtCmd));
		snprintf(SlcApp.CustomAtCmd, sizeof(SlcApp.CustomAtCmd), buff);
		SlcApp.Trigger.UploadCustomAtCmd = true;
	}
}



//-----------------------------------------------------------------------------------//
// upload or resource related function and macro declaration
//-----------------------------------------------------------------------------------//
#define SLCAPP_ISSUBSCRIBED_ALLDATA()			(SlcLwm2mDate_PowerAll_IsSubscribed() & SlcLwm2mDate_LightAll_IsSubscribed())

// #define SLCAPP_UPLOADDATA_ALLDATA()				do { \
// 													SlcApp_Upload_PowerVol(); \
// 													SlcApp_Upload_PowerFreq(); \
// 													SlcApp_Upload_PowerPF(); \
// 													SlcApp_Upload_PowerCurrent(); \
// 													SlcApp_Upload_PowerWatt(); \
// 													SlcApp_Upload_PowerTotalWatt(); \
// 													SlcApp_Upload_LightDim(); \
// 													SlcApp_Upload_LightLux(); \
// 													SlcApp_Upload_McuMode(); \
// 												} while(0)
#define SLCAPP_UPLOADDATA_ALLDATA()	

// #define SLCAPP_UPLOADDATA_ALLTHRESHOLD()		do { \
// 													SlcApp_Upload_ThresholdVolatgeHigh(); \
// 													SlcApp_Upload_ThresholdVolatgeLow(); \
// 													SlcApp_Upload_ThresholdCurrentHigh(); \
// 													SlcApp_Upload_ThresholdCurrentLow(); \
// 													SlcApp_Upload_ThresholdFreqHigh(); \
// 													SlcApp_Upload_ThresholdFreqLow(); \
// 													SlcApp_Upload_ThresholdPfHigh(); \
// 													SlcApp_Upload_ThresholdPfLow(); \
// 													SlcApp_Upload_ThresholdWattHigh(); \
// 													SlcApp_Upload_ThresholdWattLow(); \
// 													SlcApp_Upload_ThresholdLuxHigh(); \
// 													SlcApp_Upload_ThresholdLuxLow(); \
// 												} while(0)
#define SLCAPP_UPLOADDATA_ALLTHRESHOLD()

void SlcApp_Upload_NodeRegister(void)
{
	SlcLwm2mDate_UploadNodeRegister(SlcAppConfig.config.node_registered);
	wait(0.2);
	SlcApp_KickAppTimer();
}

void SlcApp_Upload_Config(void)
{
	SlcLwm2mDate_UploadConfig(SlcAppConfig.buff, sizeof(SlcAppConfig.buff));
	wait(0.2);
	SlcApp_KickAppTimer();
}

// void SlcApp_Upload_ThresholdVolatgeHigh(void)
// {
// 	SlcLwm2mDate_UploadThresholdVolatgeHigh(SlcAppTh.VoltageHigh);
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_ThresholdVolatgeLow(void)
// {
// 	SlcLwm2mDate_UploadThresholdVolatgeLow(SlcAppTh.VoltageLow);
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_ThresholdCurrentHigh(void)
// {
// 	SlcLwm2mDate_UploadThresholdCurrentHigh(SlcAppTh.CurrentHigh);
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_ThresholdCurrentLow(void)
// {
// 	SlcLwm2mDate_UploadThresholdCurrentLow(SlcAppTh.CurrentLow);
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_ThresholdFreqHigh(void)
// {
// 	SlcLwm2mDate_UploadThresholdFreqHigh(SlcAppTh.FreqHigh);
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_ThresholdFreqLow(void)
// {
// 	SlcLwm2mDate_UploadThresholdFreqLow(SlcAppTh.FreqLow);
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_ThresholdPfHigh(void)
// {
// 	SlcLwm2mDate_UploadThresholdPfHigh(SlcAppTh.PfHigh);
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_ThresholdPfLow(void)
// {
// 	SlcLwm2mDate_UploadThresholdPfLow(SlcAppTh.PfLow);
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_ThresholdWattHigh(void)
// {
// 	SlcLwm2mDate_UploadThresholdWattHigh(SlcAppTh.WattHigh);
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_ThresholdWattLow(void)
// {
// 	SlcLwm2mDate_UploadThresholdWattLow(SlcAppTh.WattLow);
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_ThresholdLuxHigh(void)
// {
// 	SlcLwm2mDate_UploadThresholdLuxHigh(SlcAppTh.LuxHigh);
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_ThresholdLuxLow(void)
// {
// 	SlcLwm2mDate_UploadThresholdLuxLow(SlcAppTh.LuxLow);
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
void SlcApp_Upload_Config_WatchdogSysSec(void)
{
	SlcLwm2mDate_UploadConfigWatchdogSysSec(SlcAppConfig.config.sys_watchdog_timeout_sec);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Config_WatchdogRegSec(void)
{
	SlcLwm2mDate_UploadConfigWatchdogRegSec(SlcAppConfig.config.reg_watchdog_timeout_sec);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Config_WatchdogLightSensorSec(void)
{
	SlcLwm2mDate_UploadConfigWatchdogLightSensorSec(SlcAppConfig.config.light_sensor_timeout_sec);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Config_PeriodFirstUploadSec(void)
{
	SlcLwm2mDate_UploadConfigPeriodFirstUploadSec(SlcAppConfig.config.first_upload_procedure.bits.cycle_time);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Config_PeriodCycleUploadSec(void)
{
	SlcLwm2mDate_UploadConfigPeriodCycleUploadSec(SlcAppConfig.config.first_upload_procedure.bits.cycle_time);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Config_PeriodNodeRegisterSec(void)
{
	SlcLwm2mDate_UploadConfigPeriodNodeRegisterSec(SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Config_PeriodStatusSec(void)
{
	SlcLwm2mDate_UploadConfigPeriodStatusSec(SlcAppConfig.config.cycle_upload_status.bits.cycle_time);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Config_PeriodDataSec(void)
{
	SlcLwm2mDate_UploadConfigPeriodDataSec(SlcAppConfig.config.cycle_upload_data.bits.cycle_time);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Config_DeregKeepOnline(void)
{
	SlcLwm2mDate_UploadConfigDeregKeepOnline(SlcAppConfig.config.deregister.bits.keep_online);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Config_DeregKeepOnlineTimeSec(void)
{
	SlcLwm2mDate_UploadConfigDeregKeepOnlineTimeSec(SlcAppConfig.config.deregister.bits.keep_online_time);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Status(void)
{
	SlcLwm2mDate_UploadStatus((int64_t)SlcAppStatus.status);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_NodeVersion(void)
{
	char strbuff[50];
	memset(strbuff, 0, sizeof(strbuff));
	snprintf(strbuff, sizeof(strbuff), "%d.%d.%d-%s-%s", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB, __DATE__, __TIME__);
	SlcLwm2mDate_UploadNodeVersion((uint8_t *)strbuff, strlen(strbuff));
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_McuVersion(void)
{
	SlcLwm2mDate_UploadMcuVersion((int64_t)SlcCtrl_GetMcuVersion());
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Imei(void)
{
	char strbuff[50];
	memset(strbuff, 0, sizeof(strbuff));
	CellularExt_GetImei(strbuff, sizeof(strbuff));
	SlcLwm2mDate_UploadImei((uint8_t *)strbuff, strlen(strbuff));
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Imsi(void)
{
	char strbuff[50];
	memset(strbuff, 0, sizeof(strbuff));
	CellularExt_GetImsi(strbuff, sizeof(strbuff));
	SlcLwm2mDate_UploadImsi((uint8_t *)strbuff, strlen(strbuff));
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Rssi(void)
{
	int rssi, ber;
	CellularExt_GetSignalQuality(&rssi, &ber);
	SlcLwm2mDate_UploadRssi((int64_t)rssi);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_ModFwVersion(void)
{
	char strbuff[50];
	memset(strbuff, 0, sizeof(strbuff));
	CellularExt_GetFwVersion(strbuff, sizeof(strbuff));
	SlcLwm2mDate_UploadModFwVer((uint8_t *)strbuff, sizeof(strbuff));
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_CellId(void)
{
	uint32_t cellid;
	CellularExt_GetCellID(&cellid);
	SlcLwm2mDate_UploadCellId((int64_t)cellid);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Rsrp(void)
{
	int rsrp, rsrq, snr;
	CellularExt_GetSignalStrength(&rsrp, &rsrq, &snr);
	SlcLwm2mDate_UploadRsrp((int64_t)rsrp);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Rsrq(void)
{
	int rsrp, rsrq, snr;
	CellularExt_GetSignalStrength(&rsrp, &rsrq, &snr);
	SlcLwm2mDate_UploadRsrq((int64_t)rsrq);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Snr(void)
{
	int rsrp, rsrq, snr;
	CellularExt_GetSignalStrength(&rsrp, &rsrq, &snr);
	SlcLwm2mDate_UploadSnr((int64_t)snr);
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_Ccid(void)
{
	char strbuff[100];
	memset(strbuff, 0, sizeof(strbuff));
	CellularExt_GetCcid(strbuff, sizeof(strbuff));
	SlcLwm2mDate_UploadCcid((uint8_t *)strbuff, sizeof(strbuff));
	wait(0.2);
	SlcApp_KickAppTimer();
}
// void SlcApp_Upload_PowerVol(void)
// {
// 	SlcLwm2mDate_UploadPowerVol(SlcCtrl_GetVoltage());
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_PowerFreq(void)
// {
// 	SlcLwm2mDate_UploadPowerFreq(SlcCtrl_GetFreq());
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_PowerPF(void)
// {
// 	SlcLwm2mDate_UploadPowerPF(SlcCtrl_GetPF());
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_PowerCurrent(void)
// {
// 	SlcLwm2mDate_UploadPowerCurrent(SlcCtrl_GetCurrent());
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_PowerWatt(void)
// {
// 	SlcLwm2mDate_UploadPowerWatt(SlcCtrl_GetWatt());
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_PowerTotalWatt(void)
// {
// 	SlcLwm2mDate_UploadPowerTotalWatt(SlcCtrl_GetTotalWatt());
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_LightDim(void)
// {
// 	int i;
// 	uint8_t tmp;
// 	_SlcApp_LightDimStruct dim;
// 	SlcCtrl_GetDimLevel(dim.config.dimarray, sizeof(dim.config.dimarray));

// 	// check if all the dim values are the same
// 	tmp = dim.config.dimarray[0];
// 	for(i=0; i<sizeof(dim.config.dimarray); i++)
// 		if(tmp != dim.config.dimarray[i])
// 			break;

// 	if(i < sizeof(dim.config.dimarray)) 	// upload all the dim values if they are different
// 	{
// 		dim.config.broadcast = 0;
// 		SlcLwm2mDate_UploadLightDim(dim.buff, sizeof(dim.buff));
// 	}
// 	else									// upload only 1 dim value with broadcast flag if all the dim values are the same
// 	{
// 		dim.config.broadcast = 1;
// 		SlcLwm2mDate_UploadLightDim(dim.buff, 2);
// 	}
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_LightLux(void)
// {
// 	SlcLwm2mDate_UploadLightLux((int64_t)SlcCtrl_GetLux());
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_McuMode(void)
// {
// 	SlcApp.McuMode = SlcCtrl_GetMcuMode();
// 	SlcLwm2mDate_UploadMcuMode((bool)SlcApp.McuMode);
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
// void SlcApp_Upload_LightSchedule(void)
// {
// 	SlcLwm2mDate_UploadLightSchedule(SlcAppLightScheduleGroup.buff, sizeof(SlcAppLightScheduleGroup.buff));
// 	wait(0.2);
// 	SlcApp_KickAppTimer();
// }
void SlcApp_Upload_SignalPowerLight(void)
{
	int offset = 0;
	char strbuff[450];
	_DateTimeStruct now;

	// get rtc
	RTC_GetRealTime(&now);

	// get signal
	int rssi, ber;
	CellularExt_GetSignalQuality(&rssi, &ber);

	// get dim
	int i;
	uint8_t tmp;
	_SlcApp_LightDimStruct dim;
	SlcCtrl_GetDimLevel(dim.config.dimarray, sizeof(dim.config.dimarray));
	// check if all the dim values are the same
	tmp = dim.config.dimarray[0];
	for(i=0; i<sizeof(dim.config.dimarray); i++)
		if(tmp != dim.config.dimarray[i])
			break;
	if(i < sizeof(dim.config.dimarray)) 	// upload all the dim values if they are different
		dim.config.broadcast = 0;
	else									// upload only 1 dim value with broadcast flag if all the dim values are the same
		dim.config.broadcast = 1;

	// get cell id
	uint32_t cellid;
	CellularExt_GetCellID(&cellid);

	// get rsrp, rsrq, snr
	int rsrp, rsrq, snr;
	CellularExt_GetSignalStrength(&rsrp, &rsrq, &snr);

	// fill string buffer
	memset(strbuff, 0, sizeof(strbuff));

	// [yy-mm-dd hh:mm:ss]rssi;voltage;current;freq;pf;watt;total_watt;lux;(broadcast);dim
	// [2019-06-12 08:19:52]-71;6660.000000;55017.000000;60003.000000;2947526656.000000:1364283776.000000:72340177116200960.000000;27;(1);100
	//offset = snprintf(strbuff, sizeof(strbuff), "[%04d-%02d-%02d %02d:%02d:%02d]%d;%f;%f;%f;%f:%f:%f;%d", 
	//			now.Y, now.M, now.D, now.h, now.m, now.s,
	//			rssi,
	//			SlcCtrl_GetVoltage(), SlcCtrl_GetFreq(), SlcCtrl_GetPF(), SlcCtrl_GetCurrent(), SlcCtrl_GetWatt(), SlcCtrl_GetTotalWatt(),
	//			SlcCtrl_GetLux());

	// node_id;data_timestamp;voltage;current;freq;pf;watt;total_watt;cell_node_rssi;subg_gw_rssi;subg_node_rssi;paired;lux;broadcase;dim
	// SLC-01
	// test：    016b450fe04d00000000000100100157;2019-06-13T06:41:08Z;666.000000;70740.640625;8.426000;0.567800;33180728.000000;477445160960.000000;-69;0;0;1;462;1;100
	// lighter: 016b450fe04d00000000000100100157;2019-06-13T06:55:08Z;768.799988;313279.968750;7.403000;0.553740;25769804.000000;0.000000;-71;0;0;1;48;1;100
	//          016b450fe04d00000000000100100157;2019-06-13T06:55:13Z;820.000000;257698.046875;53.481998;0.151830;15495569.000000;0.000000;-71;0;0;1;45;1;100
	// SLC-02
	// test:    016b207d5ab7000000000001001000a7;2019-06-13T06:58:35Z;819.599976;74109.242188;8.426000;0.829600;27117246.000000;0.000000;-87;0;0;1;440;1;100
	offset = snprintf(strbuff, sizeof(strbuff), "%s;%04d-%02d-%02dT%02d:%02d:%02dZ;%f;%f;%f;%f;%f;%f;%d;0;0;%d;%d",
				Pelion_GetDeviceID(),
				now.Y, now.M, now.D, now.h, now.m, now.s,
				/*SlcCtrl_GetVoltage(), SlcCtrl_GetCurrent(), SlcCtrl_GetFreq(), SlcCtrl_GetPF(), SlcCtrl_GetWatt(), SlcCtrl_GetTotalWatt(),*/
				0,0,0,0,0,0,
				rssi,
				/*SlcCtrl_GetMcuMode(),*/
				0,
				/*SlcCtrl_GetLux()*/
				0);

	offset += snprintf(strbuff+offset, sizeof(strbuff)-offset, ";%d", dim.config.broadcast);
	if(dim.config.broadcast == 0)
	{
		for(i=0; i<sizeof(dim.config.dimarray); i++)
			offset += snprintf(strbuff+offset, sizeof(strbuff)-offset, ";%d", dim.config.dimarray[i]);
	}
	else
	{		
		offset += snprintf(strbuff+offset, sizeof(strbuff)-offset, ";%d", dim.config.dimarray[0]);
	}

	offset = snprintf(strbuff+offset, sizeof(strbuff), ";%d;%d;%d;%d",
				cellid, rsrp, rsrq, snr);

	// upload resource to pelion
	SlcLwm2mDate_UploadSignalPowerLight((uint8_t *)strbuff, strlen(strbuff));
	wait(0.2);
	SlcApp_KickAppTimer();
}
void SlcApp_Upload_CustomAtCmd(void)
{
	char strbuff[SLC_RESOURCE_MAX_AT_RSP_STRING_SIZE];
	
	if(SlcApp.CustomAtCmdTooLong)
	{
		snprintf(strbuff, sizeof(strbuff), "Cmd Size Too Long (over spec %d)", sizeof(SlcApp.CustomAtCmd));
	}
	else
	{
		memset(strbuff, 0, sizeof(strbuff));
		CellularExt_CustomAtCmd(SlcApp.CustomAtCmd, strbuff, sizeof(strbuff));
	}
	SlcLwm2mDate_UploadCustomAtCmd((uint8_t *)strbuff, strlen(strbuff));
	wait(0.2);
	SlcApp_KickAppTimer();
}


//-----------------------------------------------------------------------------------//
// timer functions
//-----------------------------------------------------------------------------------//
static void SlcApp_TimerLaunchUploadLaunch(bool random, uint16_t interval, uint8_t random_diff)
{
	uint32_t ms;
	if(random && random_diff)
		ms = ((uint32_t)interval*1000) + (rand() % ((uint32_t)random_diff*1000)) + 1;
	else
		ms = ((uint32_t)interval*1000) + 1;
	APP_DEBUG("SlcApp_TimerLaunchUpload (%d+%d)s-> %dms", interval, random_diff, ms);
	SlcApp_TimerLaunchUpload.start(ms);
}

static void SlcApp_TimerLaunchUploadFxn(void const *n)
{
	_DateTimeStruct now;
	RTC_GetRealTime(&now);

	APP_DEBUG("trigger: LaunchUpload @ %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
	SlcApp.Trigger.LaunchUpload = true;
}

static void SlcApp_TimerTerminateUploadFxn(void const *n)
{
	_DateTimeStruct now;
	RTC_GetRealTime(&now);

	APP_DEBUG("trigger: TerminateUpload @ %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
	SlcApp.Trigger.TerminateUpload = true;
	SlcApp.TimerTerminateUpload = false;
}

static void SlcApp_TimerNodeRegisterLaunch(bool random, uint16_t interval, uint8_t random_diff)
{
	uint32_t ms;
	if(random && random_diff)
		ms = ((uint32_t)interval*1000) + (rand() % ((uint32_t)random_diff*1000)) + 1;
	else
		ms = ((uint32_t)interval*1000) + 1;
	APP_DEBUG("SlcApp_TimerNodeRegisterLaunch (%d+%d)s-> %dms", interval, random_diff, ms);
	SlcApp_TimerNodeRegister.start(ms);
}

static void SlcApp_TimerNodeRegisterFxn(void const *n)
{
	SlcApp.Trigger.UploadNodeRegister = true;
	SlcApp.TimerNodeRegister = false;
}

static void SlcApp_TimerStatusLaunch(bool random, uint16_t interval, uint8_t random_diff)
{
	uint32_t ms;
	if(random && random_diff)
		ms = ((uint32_t)interval*1000) + (rand() % ((uint32_t)random_diff*1000)) + 1;
	else
		ms = ((uint32_t)interval*1000) + 1;
	APP_DEBUG("SlcApp_TimerStatusLaunch (%d+%d)s-> %dms", interval, random_diff, ms);
	SlcApp_TimerStatus.start(ms);
}

static void SlcApp_TimerStatusFxn(void const *n)
{
	_DateTimeStruct now;
	RTC_GetRealTime(&now);

	APP_DEBUG("trigger: UploadStatus @ %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
	SlcApp.Trigger.UploadStatus = true;
	SlcApp.TimerStatus = false;
}

static void SlcApp_TimerDataLaunch(bool random, uint16_t interval, uint8_t random_diff)
{
	uint32_t ms;
	if(random && random_diff)
		ms = ((uint32_t)interval*1000) + (rand() % ((uint32_t)random_diff*1000)) + 1;
	else
		ms = ((uint32_t)interval*1000) + 1;
	APP_DEBUG("SlcApp_TimerDataLaunch  (%d+%d)s-> %dms", interval, random_diff, ms);
	SlcApp_TimerData.start(ms);
}

static void SlcApp_TimerDataFxn(void const *n)
{
	_DateTimeStruct now;
	RTC_GetRealTime(&now);

	APP_DEBUG("trigger: UploadData @ %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);	
	SlcApp.Trigger.UploadData = true;
	SlcApp.TimerData = false;
}

static void SlcApp_TimerLightSensorModeFxn(void const *n)
{
	_DateTimeStruct now;
	RTC_GetRealTime(&now);

	APP_DEBUG("trigger: SPI_LIGHT_SENSOR_MODE @ %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
	SlcCtrl_SetMcuMode(SPI_LIGHT_SENSOR_MODE);
	SlcApp.TimerLightSensor = false;
}

static void SlcApp_TimerSystemWatchdogFxn(void const *n)
{
	_DateTimeStruct now;
	RTC_GetRealTime(&now);

	SlcApp.TimerSystemWatchdog = false;
	
	APP_DEBUG("!!!!!!!! SlcApp_TimerSystemWatchdogFxn !!!!!!!!!!!!!!!!!!!");
	APP_DEBUG("@ %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
	OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
}

static void SlcApp_TimerThreadWatchdogFxn(void const *n)
{
	_DateTimeStruct now;
	RTC_GetRealTime(&now);

	SlcApp.TimerThreadWatchdog = false;
	
	APP_DEBUG("!!!!!!!! SlcApp_TimerThreadWatchdogFxn !!!!!!!!!!!!!!!!!!!");
	APP_DEBUG("@ %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
	OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
}

void SlcApp_KickAppTimer(void)
{
	//APP_DEBUG("reset light-sensor-mode timer and system-watchdog timer");
	SlcCtrl_SetMcuMode(SPI_PAIR_MODE);
	SLCAPP_RELAUNCHTIMER_LIGHTSENSORMODE();
	SLCAPP_RELAUNCHTIMER_SYSTEMWATCHDOG();
}

static void SlcApp_KickThreadTimer(void)
{
	//APP_DEBUG("reset thread-watchdog timer");
	SLCAPP_RELAUNCHTIMER_THREADWATCHDOG();
}


//-----------------------------------------------------------------------------------//
// scenario flow and related functions
//-----------------------------------------------------------------------------------//
static void SlcApp_Update_UploadTimeStamp(time_t timestamp)
{
	APP_DEBUG("timestamp %lld", timestamp);
	DataStore_WriteFile(SLCDATASTORE_UPLOADTIMESTAMP_FILE_NAME, (uint8_t *)&timestamp, sizeof(timestamp), false);
}

void SlcApp_ResetConfig(void)
{	
	SlcAppConfig.config.version										= DEFAULT_CONFIG_VERSION;
	SlcAppConfig.config.store_node_registered						= DEFAULT_STORE_NODE_REGISTERED;

	SlcAppConfig.config.sys_watchdog_timeout_sec					= DEFAULT_SYS_WATCHDOG_TIMEOUT_SEC;
	SlcAppConfig.config.reg_watchdog_timeout_sec					= DEFAULT_REG_WATCHDOG_TIMEOUT_SEC;
	SlcAppConfig.config.light_sensor_timeout_sec					= DEFAULT_LIGHT_SENSOR_TIMEOUT_SEC;

	SlcAppConfig.config.first_upload_procedure.bits.random			= DEFAULT_UPLOAD_RAMDON;
	SlcAppConfig.config.first_upload_procedure.bits.cycle_time		= DEFAULT_FIRST_UPLOAD_PROCEDURE_TIME_SEC;
	SlcAppConfig.config.first_upload_procedure.bits.diff_time		= DEFAULT_FIRST_UPLOAD_PROCEDURE_RANDOM_RANGE_SEC;	

	SlcAppConfig.config.cycle_upload_procedure.bits.random			= DEFAULT_UPLOAD_RAMDON;
	SlcAppConfig.config.cycle_upload_procedure.bits.cycle_time		= DEFAULT_CYCLE_UPLOAD_PROCEDURE_TIME_SEC;
	SlcAppConfig.config.cycle_upload_procedure.bits.diff_time		= DEFAULT_CYCLE_UPLOAD_PROCEDURE_RANDOM_RANGE_SEC;	

	SlcAppConfig.config.cycle_upload_noderegister.bits.random		= DEFAULT_UPLOAD_RAMDON;	
	SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time	= DEFAULT_CYCLE_UPLOAD_NODEREGISTER_TIME_SEC;
	SlcAppConfig.config.cycle_upload_noderegister.bits.diff_time	= DEFAULT_CYCLE_UPLOAD_NODEREGISTER_RANDOM_RANGE_SEC;	
	
	SlcAppConfig.config.cycle_upload_status.bits.random				= DEFAULT_UPLOAD_RAMDON;
	SlcAppConfig.config.cycle_upload_status.bits.cycle_time			= DEFAULT_CYCLE_UPLOAD_STATUS_TIME_SEC;
	SlcAppConfig.config.cycle_upload_status.bits.diff_time			= DEFAULT_CYCLE_UPLOAD_STATUS_RANDOM_RANGE_SEC;

	SlcAppConfig.config.cycle_upload_data_enable					= DEFAULT_UPLOAD_DATA;
	SlcAppConfig.config.cycle_upload_data.bits.cycle_time			= DEFAULT_CYCLE_UPLOAD_DATA_TIME_SEC;
	SlcAppConfig.config.cycle_upload_data.bits.random				= DEFAULT_UPLOAD_RAMDON;
	SlcAppConfig.config.cycle_upload_data.bits.diff_time			= DEFAULT_CYCLE_UPLOAD_DATA_RANDOM_RANGE_SEC;
	
	SlcAppConfig.config.deregister.bits.keep_online					= DEFAULT_KEEP_ONLINE;
	SlcAppConfig.config.deregister.bits.disconnect_type				= DEFAULT_DISCONNECT_TYPE;
	SlcAppConfig.config.deregister.bits.keep_online_time			= DEFAULT_KEEP_ONLINE_TIME;

	#ifdef _NO_CONFIGDATA_STORAGE
	SlcAppConfig.config.node_registered								= true;
	#else
	SlcAppConfig.config.node_registered								= DEFAULT_NODE_REGISTER;
	#endif

	memcpy(SlcAppConfigBackup.buff, SlcAppConfig.buff, sizeof(SlcAppConfigBackup.buff));
	//APP_DUMP("reset config", i, sizeof(SlcAppConfig.buff), SlcAppConfig.buff);
	
	DataStore_WriteFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, SlcAppConfig.buff, sizeof(SlcAppConfig.buff), false);
}

static void SlcApp_RestoreConfig(void)
{
	int i;
	_SlcApp_Config temp;
	int remain;
	int readsize;

	readsize = DataStore_ReadFile(SLCDATASTORE_SYSCONFIG_FILE_NAME, 0, temp.buff, sizeof(temp.buff), &remain);

	if(sizeof(temp) != sizeof(temp.buff))
	{
		APP_DEBUG("_SlcApp_Config size = %d", sizeof(temp));
		APP_DEBUG("_SlcApp_Config buff size = %d", sizeof(temp.buff));
		APP_DEBUG("_SlcApp_Config aligment issue, need to bed fix !!!!!!!!!!!!");
	}

	if( (readsize == sizeof(temp.buff)) && !remain && (temp.config.version == DEFAULT_CONFIG_VERSION))
	{
		memcpy(SlcAppConfig.buff, temp.buff, sizeof(SlcAppConfig.buff));
		memcpy(SlcAppConfigBackup.buff, SlcAppConfig.buff, sizeof(SlcAppConfigBackup.buff));
		if(!SlcAppConfig.config.store_node_registered)
			SlcAppConfig.config.node_registered = false; // reset node registered status when boot up
		APP_DEBUG("SysConfig restored");
	}
	else
	{
		SlcApp_ResetConfig();
		if(temp.config.version != DEFAULT_CONFIG_VERSION)
			APP_DEBUG("SysConfig version changed, reset to default");
		else
			APP_DEBUG("SysConfig restore failed, reset to default");
	}
	APP_DEBUG("//-----------------------------------------//");
	APP_DEBUG("version                                = %x(->%d)", SlcAppConfig.config.version, (SlcAppConfig.config.version&~TAG_VER));
	APP_DEBUG("store_node_registered                  = %d", SlcAppConfig.config.store_node_registered);
	
	APP_DEBUG("sys_watchdog_timeout_sec               = %d", SlcAppConfig.config.sys_watchdog_timeout_sec);
	APP_DEBUG("reg_watchdog_timeout_sec               = %d", SlcAppConfig.config.reg_watchdog_timeout_sec);
	APP_DEBUG("light_sensor_timeout_sec               = %d", SlcAppConfig.config.light_sensor_timeout_sec);
	
	APP_DEBUG("first_upload_procedure - random        = %d", SlcAppConfig.config.first_upload_procedure.bits.random);
	APP_DEBUG("first_upload_procedure - cycle_time    = %d", SlcAppConfig.config.first_upload_procedure.bits.cycle_time);
	APP_DEBUG("first_upload_procedure - diff_time     = %d", SlcAppConfig.config.first_upload_procedure.bits.diff_time);
	APP_DEBUG("cycle_upload_procedure - random	      = %d", SlcAppConfig.config.cycle_upload_procedure.bits.random);
	APP_DEBUG("cycle_upload_procedure - cycle_time    = %d", SlcAppConfig.config.cycle_upload_procedure.bits.cycle_time);
	APP_DEBUG("cycle_upload_procedure - diff_time     = %d", SlcAppConfig.config.cycle_upload_procedure.bits.diff_time);
	
	APP_DEBUG("cycle_upload_noderegister - random     = %d", SlcAppConfig.config.cycle_upload_noderegister.bits.random);
	APP_DEBUG("cycle_upload_noderegister - cycle_time = %d", SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time);
	APP_DEBUG("cycle_upload_noderegister - diff_time  = %d", SlcAppConfig.config.cycle_upload_noderegister.bits.diff_time);
	APP_DEBUG("cycle_upload_status - random	          = %d", SlcAppConfig.config.cycle_upload_status.bits.random);
	APP_DEBUG("cycle_upload_status - cycle_time       = %d", SlcAppConfig.config.cycle_upload_status.bits.cycle_time);
	APP_DEBUG("cycle_upload_status - diff_time        = %d", SlcAppConfig.config.cycle_upload_status.bits.diff_time);
	APP_DEBUG("cycle_upload_data - random             = %d", SlcAppConfig.config.cycle_upload_data.bits.random);
	APP_DEBUG("cycle_upload_data - cycle_time         = %d", SlcAppConfig.config.cycle_upload_data.bits.cycle_time);
	APP_DEBUG("cycle_upload_data - diff_time          = %d", SlcAppConfig.config.cycle_upload_data.bits.diff_time);
	APP_DEBUG("deregister - keep_online               = %d", SlcAppConfig.config.deregister.bits.keep_online);
	APP_DEBUG("deregister - disconnect_type           = %d", SlcAppConfig.config.deregister.bits.disconnect_type);
	APP_DEBUG("deregister - keep_online_time          = %d", SlcAppConfig.config.deregister.bits.keep_online_time);
	APP_DEBUG("cycle_upload_data_enable               = %d", SlcAppConfig.config.cycle_upload_data_enable);		
	APP_DEBUG("node_registered                        = %d", SlcAppConfig.config.node_registered);
	APP_DEBUG("//-----------------------------------------//");
	APP_DEBUG("config size: %d, config buff size: %d", sizeof(SlcAppConfig.config), sizeof(SlcAppConfig.buff));
	APP_DUMP("config dump", i, sizeof(SlcAppConfig.buff), SlcAppConfig.buff);
	APP_DEBUG("//-----------------------------------------//");
}

static void SlcApp_RestoreLightSchedule(void)
{
	_SlcApp_LightScheduleGroupStruct temp;
	int remain;
	int readsize;

	readsize = DataStore_ReadFile(SLCDATASTORE_LIGHTSCHEDULE_FILE_NAME, 0, temp.buff, sizeof(temp.buff), &remain);
	if( (readsize == sizeof(temp.buff)) && !remain)
	{
		memcpy(SlcAppLightScheduleGroup.buff, temp.buff, sizeof(SlcAppLightScheduleGroup.buff));
		APP_DEBUG("LightSchedule restored");

		int i;
		for(i=0; i<sizeof(SlcAppLightScheduleGroup.buff); i++)
		{
			if(SlcAppLightScheduleGroup.buff[i] != 0)
				break;
		}
		if(i == sizeof(SlcAppLightScheduleGroup.buff))
		{
			APP_DEBUG("File empty, remove");
			DataStore_RemoveFile(SLCDATASTORE_LIGHTSCHEDULE_FILE_NAME);
		}
		else
		{
			for(i=0; i<LIGHT_SCHEDULE_COUNT_MAX; i++)
			{
				APP_DEBUG("LighSchedule[%d] enable %d", i, SlcAppLightScheduleGroup.light_schedule[i].enable);
				APP_DEBUG("LighSchedule[%d] repeat %d", i, SlcAppLightScheduleGroup.light_schedule[i].repeat);
				APP_DEBUG("LighSchedule[%d] time %d-%d-%d(%d) %d:%d:%d", i, 
											SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.Y,
											SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.M,
											SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.D,
											SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.WD,
											SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.h,
											SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.m,
											SlcAppLightScheduleGroup.light_schedule[i].datetime.bits.s);
				APP_DEBUG("LighSchedule[%d] dim %d", i, SlcAppLightScheduleGroup.light_schedule[i].dim);
			}
		}
	}
	else
	{
		memset(SlcAppLightScheduleGroup.buff, 0, sizeof(SlcAppLightScheduleGroup.buff));
		// not to store all disable schedule
		//DataStore_WriteFile(SLCDATASTORE_LIGHTSCHEDULE_FILE_NAME, SlcAppLightScheduleGroup.buff, sizeof(SlcAppLightScheduleGroup.buff), false);
		APP_DEBUG("LightSchedule restore failed, reset");
	}
}

static void SlcApp_RestoreUploadTimeStamp(void)
{
	time_t temp;
	int remain;
	int readsize;

	readsize = DataStore_ReadFile(SLCDATASTORE_UPLOADTIMESTAMP_FILE_NAME, 0, (uint8_t *)&temp, sizeof(temp), &remain);
	if( (readsize == sizeof(temp)) && !remain)
	{
		SlcApp.UploadTimeStamp = temp;
		APP_DEBUG("UploadTimeStamp restored");
	}
	else
	{
		SlcApp.UploadTimeStamp = (time_t)0;
		APP_DEBUG("UploadTimeStamp restore failed, reset");
	}
}

static void SlcApp_RestoreThreshold(void)
{
	int remain;
	
	if(DataStore_ReadFile(SLCDATASTORE_VOLTAGE_THRESHOLD_H, 0, (uint8_t *)&SlcAppTh.VoltageHigh, sizeof(SlcAppTh.VoltageHigh), &remain) <= 0)
	{
		APP_DEBUG("Threshold-VoltageHigh restore failed, reset to default");
		SlcAppTh.VoltageHigh = DEFAULT_THRESHOLD_INVALID;
		//DataStore_WriteFile(SLCDATASTORE_VOLTAGE_THRESHOLD_H, (uint8_t *)&SlcAppTh.VoltageHigh, sizeof(SlcAppTh.VoltageHigh), false);
	}
	if(DataStore_ReadFile(SLCDATASTORE_VOLTAGE_THRESHOLD_L, 0, (uint8_t *)&SlcAppTh.VoltageLow, sizeof(SlcAppTh.VoltageLow), &remain) <= 0)
	{
		APP_DEBUG("Threshold-VoltageLow restore failed, reset to default");
		SlcAppTh.VoltageLow = DEFAULT_THRESHOLD_INVALID;
		//DataStore_WriteFile(SLCDATASTORE_VOLTAGE_THRESHOLD_L, (uint8_t *)&SlcAppTh.VoltageLow, sizeof(SlcAppTh.VoltageLow), false);
	}
	if(DataStore_ReadFile(SLCDATASTORE_CURRENT_THRESHOLD_H, 0, (uint8_t *)&SlcAppTh.CurrentHigh, sizeof(SlcAppTh.CurrentHigh), &remain) <= 0)
	{
		APP_DEBUG("Threshold-CurrentHigh restore failed, reset to default");
		SlcAppTh.CurrentHigh = DEFAULT_THRESHOLD_INVALID;
		//DataStore_WriteFile(SLCDATASTORE_CURRENT_THRESHOLD_H, (uint8_t *)&SlcAppTh.CurrentHigh, sizeof(SlcAppTh.CurrentHigh), false);
	}
	if(DataStore_ReadFile(SLCDATASTORE_CURRENT_THRESHOLD_L, 0, (uint8_t *)&SlcAppTh.CurrentLow, sizeof(SlcAppTh.CurrentLow), &remain) <= 0)
	{
		APP_DEBUG("Threshold-CurrentLow restore failed, reset to default");
		SlcAppTh.CurrentLow = DEFAULT_THRESHOLD_INVALID;
		//DataStore_WriteFile(SLCDATASTORE_CURRENT_THRESHOLD_L, (uint8_t *)&SlcAppTh.CurrentLow, sizeof(SlcAppTh.CurrentLow), false);
	}
	if(DataStore_ReadFile(SLCDATASTORE_FREQ_THRESHOLD_H, 0, (uint8_t *)&SlcAppTh.FreqHigh, sizeof(SlcAppTh.FreqHigh), &remain) <= 0)
	{
		APP_DEBUG("Threshold-FreqHigh restore failed, reset to default");
		SlcAppTh.FreqHigh = DEFAULT_THRESHOLD_INVALID;
		//DataStore_WriteFile(SLCDATASTORE_FREQ_THRESHOLD_H, (uint8_t *)&SlcAppTh.FreqHigh, sizeof(SlcAppTh.FreqHigh), false);
	}
	if(DataStore_ReadFile(SLCDATASTORE_FREQ_THRESHOLD_L, 0, (uint8_t *)&SlcAppTh.FreqLow, sizeof(SlcAppTh.FreqLow), &remain) <= 0)
	{
		APP_DEBUG("Threshold-FreqLow restore failed, reset to default");
		SlcAppTh.FreqLow = DEFAULT_THRESHOLD_INVALID;
		//DataStore_WriteFile(SLCDATASTORE_FREQ_THRESHOLD_L, (uint8_t *)&SlcAppTh.FreqLow, sizeof(SlcAppTh.FreqLow), false);
	}
	if(DataStore_ReadFile(SLCDATASTORE_PF_THRESHOLD_H, 0, (uint8_t *)&SlcAppTh.PfHigh, sizeof(SlcAppTh.PfHigh), &remain) <= 0)
	{
		APP_DEBUG("Threshold-PfHigh restore failed, reset to default");
		SlcAppTh.PfHigh = DEFAULT_THRESHOLD_INVALID;
		//DataStore_WriteFile(SLCDATASTORE_PF_THRESHOLD_H, (uint8_t *)&SlcAppTh.PfHigh, sizeof(SlcAppTh.PfHigh), false);
	}
	if(DataStore_ReadFile(SLCDATASTORE_PF_THRESHOLD_L, 0, (uint8_t *)&SlcAppTh.PfLow, sizeof(SlcAppTh.PfLow), &remain) <= 0)
	{
		APP_DEBUG("Threshold-PfLow restore failed, reset to default");
		SlcAppTh.PfLow = DEFAULT_THRESHOLD_INVALID;
		//DataStore_WriteFile(SLCDATASTORE_PF_THRESHOLD_L, (uint8_t *)&SlcAppTh.PfLow, sizeof(SlcAppTh.PfLow), false);
	}
	if(DataStore_ReadFile(SLCDATASTORE_WATT_THRESHOLD_H, 0, (uint8_t *)&SlcAppTh.WattHigh, sizeof(SlcAppTh.WattHigh), &remain) <= 0)
	{
		APP_DEBUG("Threshold-WattHigh restore failed, reset to default");
		SlcAppTh.WattHigh = DEFAULT_THRESHOLD_INVALID;
		//DataStore_WriteFile(SLCDATASTORE_WATT_THRESHOLD_H, (uint8_t *)&SlcAppTh.WattHigh, sizeof(SlcAppTh.WattHigh), false);
	}
	if(DataStore_ReadFile(SLCDATASTORE_WATT_THRESHOLD_L, 0, (uint8_t *)&SlcAppTh.WattLow, sizeof(SlcAppTh.WattLow), &remain) <= 0)
	{
		APP_DEBUG("Threshold-WattLow restore failed, reset to default");
		SlcAppTh.WattLow = DEFAULT_THRESHOLD_INVALID;
		//DataStore_WriteFile(SLCDATASTORE_WATT_THRESHOLD_L, (uint8_t *)&SlcAppTh.WattLow, sizeof(SlcAppTh.WattLow), false);
	}
	if(DataStore_ReadFile(SLCDATASTORE_LUX_THRESHOLD_H, 0, (uint8_t *)&SlcAppTh.LuxHigh, sizeof(SlcAppTh.LuxHigh), &remain) <= 0)
	{
		APP_DEBUG("Threshold-LuxHigh restore failed, reset to default");
		SlcAppTh.LuxHigh = DEFAULT_THRESHOLD_INVALID;
		//DataStore_WriteFile(SLCDATASTORE_LUX_THRESHOLD_H, (uint8_t *)&SlcAppTh.LuxHigh, sizeof(SlcAppTh.LuxHigh), false);
	}
	if(DataStore_ReadFile(SLCDATASTORE_LUX_THRESHOLD_L, 0, (uint8_t *)&SlcAppTh.LuxLow, sizeof(SlcAppTh.LuxLow), &remain) <= 0)
	{
		APP_DEBUG("Threshold-LuxLow restore failed, reset to default");
		SlcAppTh.LuxLow = DEFAULT_THRESHOLD_INVALID;
		//DataStore_WriteFile(SLCDATASTORE_LUX_THRESHOLD_L, (uint8_t *)&SlcAppTh.LuxLow, sizeof(SlcAppTh.LuxLow), false);
	}

	APP_DEBUG("//-----------------------------------------//");
	APP_DEBUG("VoltageHigh = %f", SlcAppTh.VoltageHigh);
	APP_DEBUG("VoltageLow  = %f", SlcAppTh.VoltageLow);
	APP_DEBUG("CurrentHigh = %f", SlcAppTh.CurrentHigh);
	APP_DEBUG("CurrentLow  = %f", SlcAppTh.CurrentLow);
	APP_DEBUG("FreqHigh    = %f", SlcAppTh.FreqHigh);
	APP_DEBUG("FreqLow     = %f", SlcAppTh.FreqLow);
	APP_DEBUG("PfLow       = %f", SlcAppTh.PfHigh);
	APP_DEBUG("PfLow       = %f", SlcAppTh.PfLow);
	APP_DEBUG("WattHigh    = %f", SlcAppTh.WattHigh);
	APP_DEBUG("WattLow     = %f", SlcAppTh.WattLow);
	APP_DEBUG("LuxHigh     = %d", SlcAppTh.LuxHigh);
	APP_DEBUG("LuxLow      = %d", SlcAppTh.LuxLow);
	APP_DEBUG("//-----------------------------------------//");
}

static void SlcApp_ResetStatus(void)
{
	SlcAppStatus.status					= 0;

	SlcApp.Trigger.LaunchUpload 		= false;
	SlcApp.Trigger.TerminateUpload 		= false;
	SlcApp.Trigger.UploadNodeRegister	= false;
	SlcApp.Trigger.UploadStatus			= false;
	SlcApp.Trigger.UploadData			= false;
	SlcApp.Trigger.FWUpdate				= false;
	SlcApp.Trigger.FWUpdateTerminated	= false;
	SlcApp.Trigger.UploadCustomAtCmd	= false;
	SlcApp.FirstUploadDone				= 0;
	SlcApp.FWUpdating					= false;
	SlcApp.McuMode						= SPI_LIGHT_SENSOR_MODE;
	SlcApp.TimerTerminateUpload			= false;
	SlcApp.TimerNodeRegister			= false;
	SlcApp.TimerStatus					= false;
	SlcApp.TimerData					= false;
	SlcApp.TimerLightSensor				= false;
	SlcApp.TimerSystemWatchdog			= false;
	SlcApp.TimerThreadWatchdog			= false;	
	SlcApp.CustomAtCmdTooLong			= false;
}

// support 2 date&time format, choose one of them, the other set NULL
// input full date and time means match for exactlly
// set year as 0 for yearly match check
// set year and month as 0 for monthly match check
// set year and month and day as 0 for weekly match check
// set year and month and day and weekday as 0 for daily match check
static bool SlcApp_DateTimeMatch(_SlcApp_DateTimeStruct *slcdatetime, _DateTimeStruct *datetime)
{
	//time_t timestamp;
	//struct tm *t;
	//timestamp = (time_t)time(NULL);
	//t = localtime(&timestamp);
	_DateTimeStruct now;
	RTC_GetRealTime(&now);
	
	if(schedule_debug_show)
	{
		APP_DEBUG("SlcApp_DateTimeMatch");
		//APP_DEBUG("rtc: %d-%02d-%02d (w:%d) %02d:%02d:%02d (dst:%d)\n", t->tm_year+NTP_DATETIME_BASE_YEAR, t->tm_mon+NTP_DATETIME_BASE_MONTH, t->tm_mday, t->tm_wday,
		//											t->tm_hour, t->tm_min, t->tm_sec, t->tm_isdst);
		APP_DEBUG("rtc:  %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD,
													now.h, now.m, now.s);
		if(slcdatetime)
		{
			APP_DEBUG("slcdt:%d-%02d-%02d (w:%d) %02d:%02d:%02d", slcdatetime->bits.Y+SLC_DATETIME_BASE_YEAR, slcdatetime->bits.M, slcdatetime->bits.D, slcdatetime->bits.WD,
													slcdatetime->bits.h, slcdatetime->bits.m, slcdatetime->bits.s);
		}
		if(datetime)
		{
			APP_DEBUG("dt:   %d-%02d-%02d (w:%d) %02d:%02d:%02d", datetime->Y, datetime->M, datetime->D, datetime->WD,
													datetime->h, datetime->m, datetime->s);
		}

		schedule_debug_show = false;
	}

	if(!IF_REAL_RTC(now.Y))		// date and time has not been sync yet
	{
		APP_DEBUG("SlcApp_DateTimeMatch, RTC time is not real");
		return false;
	}

	//----- date and time match exactlly -----//
	if( slcdatetime
		&& ((slcdatetime->bits.Y+SLC_DATETIME_BASE_YEAR) == now.Y)
		&& (slcdatetime->bits.M 	== now.M)
		&& (slcdatetime->bits.D 	== now.D)
		&& (slcdatetime->bits.WD 	== now.WD)
		&& (slcdatetime->bits.h 	== now.h)
		&& (slcdatetime->bits.m 	== now.m) )
	{
		APP_DEBUG("slc date and time match");
		APP_DEBUG("rtc:  %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
		return true;
	}
	else if( datetime
		&& (datetime->Y 	== now.Y)
		&& (datetime->M 	== now.M)
		&& (datetime->D 	== now.D)
		&& (datetime->WD 	== now.WD)
		&& (datetime->h 	== now.h)
		&& (datetime->m 	== now.m) )
	{
		APP_DEBUG("date and time match");
		APP_DEBUG("rtc:  %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
		return true;
	}
	//----- date and time match yearly -----//
	if( slcdatetime
		&& (slcdatetime->bits.Y 	== 255)
		&& (slcdatetime->bits.M 	== now.M)
		&& (slcdatetime->bits.D 	== now.D)
		//&& (slcdatetime->bits.WD 	== 0)
		&& (slcdatetime->bits.h 	== now.h)
		&& (slcdatetime->bits.m 	== now.m) )
	{
		APP_DEBUG("slc yearly date and time match");
		APP_DEBUG("rtc:  %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
		return true;
	}
	else if( datetime
		&& (datetime->Y 	== 255)
		&& (datetime->M 	== now.M)
		&& (datetime->D 	== now.D)
		// (datetime->WD 	== 0)
		&& (datetime->h 	== now.h)
		&& (datetime->m 	== now.m) )
	{
		APP_DEBUG("yearly date and time match");
		APP_DEBUG("rtc:  %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
		return true;
	}
	//----- date and time match monthly -----//
	if( slcdatetime
		&& (slcdatetime->bits.Y 	== 255)
		&& (slcdatetime->bits.M 	== 255)
		&& (slcdatetime->bits.D 	== now.D)
		//&& (slcdatetime->bits.WD 	== 0)
		&& (slcdatetime->bits.h 	== now.h)
		&& (slcdatetime->bits.m 	== now.m) )
	{
		APP_DEBUG("slc monthly date and time match");
		APP_DEBUG("rtc:  %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
		return true;
	}
	else if( datetime
		&& (datetime->Y 	== 255)
		&& (datetime->M 	== 255)
		&& (datetime->D 	== now.D)
		//&& (datetime->WD 	== 0)
		&& (datetime->h 	== now.h)
		&& (datetime->m 	== now.m) )
	{
		APP_DEBUG("monthly date and time match");
		APP_DEBUG("rtc:  %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
		return true;
	}
	//----- date and time match weekly -----//
	if( slcdatetime
		&& (slcdatetime->bits.Y 	== 255)
		&& (slcdatetime->bits.M 	== 255)
		&& (slcdatetime->bits.D 	== 255)
		&& (slcdatetime->bits.WD 	== now.WD)
		&& (slcdatetime->bits.h 	== now.h)
		&& (slcdatetime->bits.m 	== now.m) )
	{
		APP_DEBUG("slc weekly date and time match");
		APP_DEBUG("rtc:  %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
		return true;
	}
	else if( datetime
		&& (datetime->Y 	== 255)
		&& (datetime->M 	== 255)
		&& (datetime->D 	== 255)
		&& (datetime->WD 	== now.WD)
		&& (datetime->h 	== now.h)
		&& (datetime->m 	== now.m) )
	{
		APP_DEBUG("weekly date and time match");
		APP_DEBUG("rtc:  %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
		return true;
	}
	//----- only time match for daily -----//
	else if( slcdatetime
		&& (slcdatetime->bits.Y 	== 255)
		&& (slcdatetime->bits.M 	== 255)
		&& (slcdatetime->bits.D 	== 255)
		&& (slcdatetime->bits.WD 	== 255)
		&& (slcdatetime->bits.h 	== now.h)
		&& (slcdatetime->bits.m 	== now.m) )
	{
		APP_DEBUG("slc daily time match");
		APP_DEBUG("rtc:  %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
		return true;
	}
	else if( datetime
		&& ((datetime->Y) 	== 255)
		&& (datetime->M 	== 255)
		&& (datetime->D 	== 255)
		&& (datetime->WD	== 255)
		&& (datetime->h 	== now.h)
		&& (datetime->m 	== now.m) )
	{
		APP_DEBUG("daily time match");
		APP_DEBUG("rtc:  %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
		return true;
	}
	//----- only time match for hourly -----//
	else if( slcdatetime
		&& (slcdatetime->bits.Y 	== 255)
		&& (slcdatetime->bits.M 	== 255)
		&& (slcdatetime->bits.D 	== 255)
		&& (slcdatetime->bits.WD 	== 255)
		&& (slcdatetime->bits.h 	== 255)
		&& (slcdatetime->bits.m 	== now.m) )
	{
		APP_DEBUG("slc hourly time match");
		APP_DEBUG("rtc:  %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
		return true;
	}
	else if( datetime
		&& ((datetime->Y) 	== 255)
		&& (datetime->M 	== 255)
		&& (datetime->D 	== 255)
		&& (datetime->WD 	== 255)
		&& (datetime->h 	== 255)
		&& (datetime->m 	== now.m) )
	{
		APP_DEBUG("hourly time match");
		APP_DEBUG("rtc:  %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
		return true;
	}
	else
		return false;
}

// threshold invalid and not to be checked if threshold value is DEFAULT_THRESHOLD_INVALID
static bool SlcApp_CheckOutOfSpec(float value, float th_h, float th_l)
{
	if( (th_h != DEFAULT_THRESHOLD_INVALID) && (th_l != DEFAULT_THRESHOLD_INVALID) )
	{
		if((value > th_h) || (value < th_l))
			return true;
	}
	else if(th_h != DEFAULT_THRESHOLD_INVALID)
	{
		if(value > th_h)
			return true;
	}
	else if(th_l != DEFAULT_THRESHOLD_INVALID)
	{
		if(value < th_l)
			return true;
	}

	return false;
}

static void SlcApp_ThreadFxn(void const *args)
{
	int disconnect_waitcnt;
	int i;
	_SlcApp_AppState		appstate;
	_SlcApp_DevRegState		devregstate;
	
	APP_DEBUG("SlcApp ThreadFxn");
	
	appstate = SLCAPP_APPSTATE_READY_TO_IDLE;
	APP_DEBUG("jump to SLCAPP_APPSTATE_READY_TO_IDLE");
	while(1)
	{
		//-------------------------------------------------------//
		// state machine for node scenario control
		//-------------------------------------------------------//
		switch(appstate)
		{
			case SLCAPP_APPSTATE_READY_TO_IDLE:
				if(SlcApp.FirstUploadDone == 0)
				{
					SLCAPP_LAUNCHTIMER_1ST_LAUNCHUPLOAD();
				}
				else if(SlcApp.FirstUploadDone < REDUCE_UPLOAD_PERIOD_TIMES)
				{
					SLCAPP_LAUNCHTIMER_REDUCE_PERIOD_LAUNCHUPLOAD();
				}
				else
				{
					SLCAPP_LAUNCHTIMER_LAUNCHUPLOAD();
				}
				SLCAPP_LED_IDLE();
				appstate = SLCAPP_APPSTATE_IDLE;
				APP_DEBUG("jump to SLCAPP_APPSTATE_IDLE");
				break;

			case SLCAPP_APPSTATE_IDLE:
				if(SlcApp.Trigger.LaunchUpload)
				{
					#ifndef _MBEDCLOUDCLIENT_PAUSERESUME
					APP_DEBUG("reset resource subscribed state for client open/close");
					SlcLwm2mDate_ResetResourceSubscribed();
					#endif
					Pelion_ToRestart();
					SLCAPP_RELAUNCHTIMER_REG_WATCHDOG();
					SLCAPP_LED_NETWORK_CONNECTING();
					appstate = SLCAPP_APPSTATE_WAIT_NETWORK_CONNECT;
					APP_DEBUG("jump to SLCAPP_APPSTATE_WAIT_NETWORK_CONNECT");
					SlcApp.Trigger.LaunchUpload = false;
				}
				else
				{
					APP_DEBUG("waiting update process launch...");
					wait(DEBUG_DISPLAY_PERIOD_LONG_SEC);
				}
				break;
				
			case SLCAPP_APPSTATE_WAIT_NETWORK_CONNECT:
				{
					_NetState netstat;
					netstat = Network_GetState();
					switch(netstat)
					{
						case NETSTATE_CONNECTED:
							appstate = SLCAPP_APPSTATE_INIT;
							APP_DEBUG("jump to SLCAPP_APPSTATE_INIT");
							break;

						case NETSTATE_ERROR:
						case NETSTATE_ERROR_REBOOT:
							APP_DEBUG("jump to SLCAPP_APPSTATE_TO_PELION_DEREG caused by network error");
							appstate = SLCAPP_APPSTATE_TO_PELION_DEREG;
							SlcApp.Trigger.TerminateUpload = false;
							break;

						case NETSTATE_CONNECTING:
						case NETSTATE_DISCONNECTED:
						case NETSTATE_RAIDISCONNECTED:
						case NETSTATE_DETACHDISCONNECTED:
						default:
							if(SlcApp.Trigger.TerminateUpload)
							{
								APP_DEBUG("jump to SLCAPP_APPSTATE_TO_PELION_DEREG caused by timeout");
								appstate = SLCAPP_APPSTATE_TO_PELION_DEREG;
								SlcApp.Trigger.TerminateUpload = false;
							}
							else
							{
								APP_DEBUG("waiting network established...");
								wait(DEBUG_DISPLAY_PERIOD_SEC);
							}
							break;
					}
				}
				break;

			case SLCAPP_APPSTATE_INIT:
				// initialize status
				SlcApp.Trigger.UploadNodeRegister 	= false;
				SLCAPP_RELAUNCHTIMER_REG_WATCHDOG();
				SLCAPP_LED_PELION_REGISTERING();
				appstate = SLCAPP_APPSTATE_WAIT_PELION_REG;
				APP_DEBUG("jump to SLCAPP_APPSTATE_WAIT_PELION_REG");
				break;
			
			case SLCAPP_APPSTATE_WAIT_PELION_REG:
				if(Pelion_IsRegistered() && !Pelion_IsPaused())
				{
					SLCAPP_RELAUNCHTIMER_REG_WATCHDOG();
					appstate = SLCAPP_APPSTATE_WAIT_RESOURCE_SUBSCRIBED;
					SLCAPP_LED_RESOURCE_SUBSCRIBING();
					APP_DEBUG("jump to SLCAPP_APPSTATE_WAIT_RESOURCE_SUBSCRIBED");
				}
				else if(SlcApp.Trigger.TerminateUpload)
				{
					APP_DEBUG("jump to SLCAPP_APPSTATE_TO_PELION_DEREG");
					appstate = SLCAPP_APPSTATE_TO_PELION_DEREG;
					SlcApp.Trigger.TerminateUpload = false;
				}
				else
				{
					APP_DEBUG("waiting pelion registered...");
					wait(DEBUG_DISPLAY_PERIOD_SEC);
				}
				break;

			case SLCAPP_APPSTATE_WAIT_RESOURCE_SUBSCRIBED:
				if(SlcLwm2mDate_IsAllResSubscribed())
				{
					//SLCAPP_RELAUNCHTIMER_REG_WATCHDOG();
					SLCAPP_RELAUNCHTIMER_NODEREG_WATCHDOG();
					//SLCAPP_LED_DEVICE_REGISTERING();
					appstate = SLCAPP_APPSTATE_WAIT_DEV_REG;
					devregstate = SLCAPP_DEVREGSTATE_INIT;
					APP_DEBUG("jump to SLCAPP_APPSTATE_WAIT_DEV_REG - SLCAPP_DEVREGSTATE_INIT");
				}
				else if(SlcApp.Trigger.TerminateUpload)
				{
					APP_DEBUG("jump to SLCAPP_APPSTATE_TO_PELION_DEREG");
					appstate = SLCAPP_APPSTATE_TO_PELION_DEREG;
					SlcApp.Trigger.TerminateUpload = false;
				}
				#ifdef _NO_DISCONNECT_WHEN_FW_UPDATING
				else if(SlcApp.Trigger.FWUpdate)
				{
					APP_DEBUG("stop timer and wait for fw update complete");
					SLCAPP_STOPTIMER_REG_WATCHDOG();
					SlcApp.FWUpdating = true;
					SlcApp.Trigger.FWUpdate = false;
				}
				else if(SlcApp.Trigger.FWUpdateTerminated)
				{
					APP_DEBUG("fw update terminated and relaunch timer");
					SLCAPP_RELAUNCHTIMER_REG_WATCHDOG();
					SlcApp.FWUpdating = false;
					SlcApp.Trigger.FWUpdateTerminated = false;
				}
				#endif
				else
				{
					APP_DEBUG("waiting resources subscribed...");
					int res_cnt;
					bool *ptmp = SlcLwm2mDate_ResSubscribedStatus(&res_cnt);
					APP_DUMP("Subscribed", i, res_cnt, ptmp);
					//Main_PrintHeapStack(TAG);
					wait(DEBUG_DISPLAY_PERIOD_SEC);
				}
				break;
				
			case SLCAPP_APPSTATE_WAIT_DEV_REG:
				switch(devregstate)
				{
					case SLCAPP_DEVREGSTATE_INIT:
						devregstate = SLCAPP_DEVREGSTATE_WAIT_REG;
						if(!SlcAppConfig.config.node_registered)
						{
							SlcApp_Upload_NodeRegister();
							SLCAPP_LAUNCHTIMER_NODEREGISTER();
						}
						APP_DEBUG("jump to SLCAPP_APPSTATE_WAIT_DEV_REG - SLCAPP_DEVREGSTATE_WAIT_REG");
						break;
					case SLCAPP_DEVREGSTATE_WAIT_REG:
						if(SlcAppConfig.config.node_registered)
						{
							//SLCAPP_STOPTIMER_REG_WATCHDOG();
							SLCAPP_STOPTIMER_TERMINATEUPLOAD();
							devregstate = SLCAPP_DEVREGSTATE_DONE;
							APP_DEBUG("jump to SLCAPP_APPSTATE_WAIT_DEV_REG - SLCAPP_DEVREGSTATE_DONE");
						}
						else if(SlcApp.Trigger.UploadNodeRegister)
						{
							SlcApp_Upload_NodeRegister();
							SLCAPP_LAUNCHTIMER_NODEREGISTER();
							SlcApp.Trigger.UploadNodeRegister = false;
						}
						else if(SlcApp.Trigger.TerminateUpload)
						{
							appstate = SLCAPP_APPSTATE_TO_PELION_DEREG;
							SlcApp.Trigger.TerminateUpload = false;
							APP_DEBUG("jump to SLCAPP_APPSTATE_TO_PELION_DEREG");
						}
						#ifdef _NO_DISCONNECT_WHEN_FW_UPDATING
						else if(SlcApp.Trigger.FWUpdate)
						{
							//SLCAPP_STOPTIMER_REG_WATCHDOG();
							SLCAPP_STOPTIMER_TERMINATEUPLOAD();
							SlcApp.FWUpdating = true;
							SlcApp.Trigger.FWUpdate = false;
							APP_DEBUG("stop timer and wait for fw update complete");
						}
						else if(SlcApp.Trigger.FWUpdateTerminated)
						{
							//SLCAPP_RELAUNCHTIMER_REG_WATCHDOG();
							SLCAPP_RELAUNCHTIMER_NODEREG_WATCHDOG();
							SlcApp.FWUpdating = false;
							SlcApp.Trigger.FWUpdateTerminated = false;
							APP_DEBUG("fw update terminated and relaunch timer");
						}
						#endif
						else
						{
							APP_DEBUG("waiting node device registered...");
							wait(DEBUG_DISPLAY_PERIOD_SEC);
						}
						break;
					case SLCAPP_DEVREGSTATE_DONE:
						// use reg-watchdog to wait all resources subscribed
						//if(!SlcAppConfig.config.deregister.bits.keep_online && !SlcApp.FWUpdating)
						//{
						//	// prepare to deregister from pelion and disconnect
						//	APP_DEBUG("prepare timer to deregister from pelion and disconnect");
						//	SLCAPP_RELAUNCHTIMER_TERMINATEUPLOAD();
						//}
						SLCAPP_LED_UPLOAD_STRANDBY();
						appstate = SLCAPP_APPSTATE_READY_TO_STANDBY;
						APP_DEBUG("jump to SLCAPP_APPSTATE_READY_TO_STANDBY");
						break;
				}
				break;
				
			case SLCAPP_APPSTATE_READY_TO_STANDBY:
				{
				time_t timestamp = RTC_GetRealTimeStamp();
				//APP_DEBUG("timestamp last=%lld, now=%lld, diff=%lld", SlcApp.UploadTimeStamp, timestamp, timestamp-SlcApp.UploadTimeStamp);
				if(!SlcAppConfigBackup.config.node_registered
					|| (SlcApp.FirstUploadDone==0) )
				//	|| ((timestamp-SlcApp.UploadTimeStamp) > TIME_DAY) )	// reserved for upload longterm
				{
					// stop timer for upload node_register
					//APP_DEBUG("-> upload node_register");
					//SlcApp_Upload_NodeRegister();
					APP_DEBUG("-> stop timer of upload node_register");
					SLCAPP_STOPTIMER_NODEREGISTER();

					// upload all resources
					APP_DEBUG("upload all resources");
					APP_DEBUG("-> upload status");
					SlcApp_Upload_Status();
					APP_DEBUG("-> upload config");
					SlcApp_Upload_Config();
					// APP_DEBUG("-> upload light schedule");
					// SlcApp_Upload_LightSchedule();
					APP_DEBUG("-> upload node version");
					SlcApp_Upload_NodeVersion();
					APP_DEBUG("-> upload imei");
					SlcApp_Upload_Imei();
					APP_DEBUG("-> upload imsi");
					SlcApp_Upload_Imsi();
					APP_DEBUG("-> upload mod fw version");
					SlcApp_Upload_ModFwVersion();
					//APP_DEBUG("-> upload rssi");
					//SlcApp_Upload_Rssi();
					APP_DEBUG("-> upload mcu version");
					SlcApp_Upload_McuVersion();
					//APP_DEBUG("-> upload cell id");
					//SlcApp_Upload_CellId();
					//APP_DEBUG("-> upload rsrp");
					//SlcApp_Upload_Rsrp();
					//APP_DEBUG("-> upload rsrq");
					//SlcApp_Upload_Rsrq();
					//APP_DEBUG("-> upload snr");
					//SlcApp_Upload_Snr();
					APP_DEBUG("-> upload ccid");
					SlcApp_Upload_Ccid();
					//APP_DEBUG("upload all data");
					//SLCAPP_UPLOADDATA_ALLDATA();
					APP_DEBUG("upload all threshold");
					SLCAPP_UPLOADDATA_ALLTHRESHOLD();
					
					// APP_DEBUG("-> upload light dim");
					// SlcApp_Upload_LightDim();
					// APP_DEBUG("-> upload light schedule");
					// SlcApp_Upload_LightSchedule();

					//SlcApp_Upload_SignalPowerLight();
					APP_DEBUG("-> upload signal power light");
					SlcApp_Upload_SignalPowerLight(); // modifdy to upload once for each upload process

					// launch timer for upload periodically
					APP_DEBUG("-> launch timer of upload status");
					SLCAPP_LAUNCHTIMER_STATUS();
					if(SlcAppConfig.config.cycle_upload_data_enable)
					{
						APP_DEBUG("-> launch timer of upload data");
						SLCAPP_RELAUNCHTIMER_DATA();
					}

					APP_DEBUG("-> update time stamp");
					SlcAppConfigBackup.config.node_registered = SlcAppConfig.config.node_registered;
					SlcApp_Update_UploadTimeStamp(timestamp);
					SlcApp.UploadTimeStamp = timestamp;
				}
				
				if(!SlcAppConfig.config.deregister.bits.keep_online && !SlcApp.FWUpdating)
				{
					// prepare to deregister from pelion and disconnect
					APP_DEBUG("prepare timer %d secs to deregister from pelion and disconnect %d", SlcAppConfig.config.deregister.bits.keep_online_time, SlcAppConfig.config.deregister.bits.disconnect_type);
					SLCAPP_RELAUNCHTIMER_TERMINATEUPLOAD();
				}
				if(SlcApp.FirstUploadDone < REDUCE_UPLOAD_PERIOD_TIMES)
					SlcApp.FirstUploadDone++;
				appstate = SLCAPP_APPSTATE_STANDBY;
				APP_DEBUG("jump to SLCAPP_APPSTATE_STANDBY");
				}
				break;
				
			case SLCAPP_APPSTATE_STANDBY:
				// update system watchdog timer
				if(SlcAppConfig.config.sys_watchdog_timeout_sec != SlcAppConfigBackup.config.sys_watchdog_timeout_sec)
				{
					APP_DEBUG("to update system watchdog timer");
					SLCAPP_RELAUNCHTIMER_SYSTEMWATCHDOG();
					SlcAppConfigBackup.config.sys_watchdog_timeout_sec = SlcAppConfig.config.sys_watchdog_timeout_sec;
				}

				// update register watchdog timer
				if(SlcAppConfig.config.reg_watchdog_timeout_sec != SlcAppConfigBackup.config.reg_watchdog_timeout_sec)
				{
					APP_DEBUG("to update register watchdog timer");
					SlcAppConfigBackup.config.reg_watchdog_timeout_sec = SlcAppConfig.config.reg_watchdog_timeout_sec;
				}
				
				// update light-sensor-mode timer
				if(SlcAppConfig.config.light_sensor_timeout_sec != SlcAppConfigBackup.config.light_sensor_timeout_sec)
				{
					APP_DEBUG("to update light sensor mode timer");
					SLCAPP_RELAUNCHTIMER_LIGHTSENSORMODE();
					SlcAppConfigBackup.config.light_sensor_timeout_sec = SlcAppConfig.config.light_sensor_timeout_sec;
				}

				// update first upload procedure timer
				if(SlcAppConfig.config.first_upload_procedure.bits.cycle_time != SlcAppConfigBackup.config.first_upload_procedure.bits.cycle_time)
				{
					APP_DEBUG("to update first upload procedure timer");
					SlcAppConfigBackup.config.first_upload_procedure.bits.cycle_time = SlcAppConfig.config.first_upload_procedure.bits.cycle_time;
					SlcAppConfigBackup.config.first_upload_procedure.bits.random = SlcAppConfig.config.first_upload_procedure.bits.random;
					SlcAppConfigBackup.config.first_upload_procedure.bits.diff_time = SlcAppConfig.config.first_upload_procedure.bits.diff_time;
				}

				// update cycle upload procedure timer
				if(SlcAppConfig.config.cycle_upload_procedure.bits.cycle_time != SlcAppConfigBackup.config.cycle_upload_procedure.bits.cycle_time)
				{
					APP_DEBUG("to update first upload procedure timer");
					SlcAppConfigBackup.config.cycle_upload_procedure.bits.cycle_time = SlcAppConfig.config.cycle_upload_procedure.bits.cycle_time;
					SlcAppConfigBackup.config.cycle_upload_procedure.bits.random = SlcAppConfig.config.cycle_upload_procedure.bits.random;
					SlcAppConfigBackup.config.cycle_upload_procedure.bits.diff_time = SlcAppConfig.config.cycle_upload_procedure.bits.diff_time;
				}

				// update node register timer
				if(SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time != SlcAppConfigBackup.config.cycle_upload_noderegister.bits.cycle_time)
				{
					APP_DEBUG("to update node register timer");
					SlcAppConfigBackup.config.cycle_upload_noderegister.bits.cycle_time = SlcAppConfig.config.cycle_upload_noderegister.bits.cycle_time;
					SlcAppConfigBackup.config.cycle_upload_noderegister.bits.random = SlcAppConfig.config.cycle_upload_noderegister.bits.random;
					SlcAppConfigBackup.config.cycle_upload_noderegister.bits.diff_time = SlcAppConfig.config.cycle_upload_noderegister.bits.diff_time;
				}

				// update status timer
				if(SlcAppConfig.config.cycle_upload_status.bits.cycle_time != SlcAppConfigBackup.config.cycle_upload_status.bits.cycle_time)
				{
					APP_DEBUG("to update status timer");
					SLCAPP_STOPTIMER_STATUS();
					SlcApp_Upload_Status();
					SLCAPP_LAUNCHTIMER_STATUS();
					SlcAppConfigBackup.config.cycle_upload_status.bits.cycle_time = SlcAppConfig.config.cycle_upload_status.bits.cycle_time;
					SlcAppConfigBackup.config.cycle_upload_status.bits.random = SlcAppConfig.config.cycle_upload_status.bits.random;
					SlcAppConfigBackup.config.cycle_upload_status.bits.diff_time = SlcAppConfig.config.cycle_upload_status.bits.diff_time;
				}
						
				// enable/disable/update data timer
				if( (SlcAppConfig.config.cycle_upload_data_enable != SlcAppConfigBackup.config.cycle_upload_data_enable)
					||(SlcAppConfig.config.cycle_upload_data.bits.random != SlcAppConfigBackup.config.cycle_upload_data.bits.random)
					||(SlcAppConfig.config.cycle_upload_data.bits.cycle_time != SlcAppConfigBackup.config.cycle_upload_data.bits.cycle_time)
					||(SlcAppConfig.config.cycle_upload_data.bits.diff_time != SlcAppConfigBackup.config.cycle_upload_data.bits.diff_time) )
				{
					if(SlcAppConfig.config.cycle_upload_data_enable)
					{
						APP_DEBUG("to enable/update data timer");
						SLCAPP_UPLOADDATA_ALLDATA();
						SLCAPP_RELAUNCHTIMER_DATA();
					}
					else
					{
						APP_DEBUG("to disable data timer");
						SLCAPP_STOPTIMER_DATA();
					}
					SlcAppConfigBackup.config.cycle_upload_data_enable = SlcAppConfig.config.cycle_upload_data_enable;
					SlcAppConfigBackup.config.cycle_upload_data.bits.cycle_time = SlcAppConfig.config.cycle_upload_data.bits.cycle_time;
					SlcAppConfigBackup.config.cycle_upload_data.bits.random = SlcAppConfig.config.cycle_upload_data.bits.random;
					SlcAppConfigBackup.config.cycle_upload_data.bits.diff_time = SlcAppConfig.config.cycle_upload_data.bits.diff_time;
				}

				// enable/disable/update deregister
				if( (SlcAppConfig.config.deregister.bits.keep_online != SlcAppConfigBackup.config.deregister.bits.keep_online)
					|| (SlcAppConfig.config.deregister.bits.keep_online_time != SlcAppConfigBackup.config.deregister.bits.keep_online_time) )
				{
					if(SlcAppConfig.config.deregister.bits.keep_online)
						SLCAPP_STOPTIMER_TERMINATEUPLOAD();
					else
						SLCAPP_RELAUNCHTIMER_TERMINATEUPLOAD();
					
					SlcAppConfigBackup.config.deregister.bits.keep_online = SlcAppConfig.config.deregister.bits.keep_online;
					SlcAppConfigBackup.config.deregister.bits.keep_online_time = SlcAppConfig.config.deregister.bits.keep_online_time;
				}

				// update dim
				if( (SlcAppLightDim.config.broadcast != SlcAppLightDimBackup.config.broadcast) 
					|| (memcmp(SlcAppLightDim.config.dimarray, SlcAppLightDimBackup.config.dimarray, sizeof(SlcAppLightDim.config.dimarray))) )
				{
					APP_DEBUG("to update dim");
					if(SlcAppLightDim.config.broadcast)
					{
						APP_DEBUG("to update dim broadcast");
						memset(SlcAppLightDim.config.dimarray, SlcAppLightDim.config.dimarray[0], sizeof(SlcAppLightDim.config.dimarray));
					}
					APP_DUMP("dim", i, sizeof(SlcAppLightDim.config.dimarray), SlcAppLightDim.config.dimarray);
					SlcCtrl_SetDimLevel(SlcAppLightDim.config.broadcast, SlcAppLightDim.config.dimarray, sizeof(SlcAppLightDim.config.dimarray));
					SlcAppLightDimBackup.config.broadcast = SlcAppLightDim.config.broadcast;
					memcpy(SlcAppLightDimBackup.config.dimarray, SlcAppLightDim.config.dimarray, sizeof(SlcAppLightDim.config.dimarray));
				}
					
				// back to non dev-register
				if(!SlcAppConfig.config.node_registered)
				{
					APP_DEBUG("to undo dev-register");
					appstate = SLCAPP_APPSTATE_WAIT_NETWORK_CONNECT;
					SlcAppConfigBackup.config.node_registered = SlcAppConfig.config.node_registered;
				}

				// to upload status
				if(SlcApp.Trigger.UploadStatus)
				{
					APP_DEBUG("to upload status");
					SlcApp_Upload_Status();
					APP_DEBUG("to upload status **********");
					SLCAPP_LAUNCHTIMER_STATUS();
					SlcApp.Trigger.UploadStatus = false;
				}

				// to upload data
				if(SlcApp.Trigger.UploadData)
				{
					APP_DEBUG("to upload data");
					//SLCAPP_UPLOADDATA_ALLDATA();
					SlcApp_Upload_SignalPowerLight();
					SLCAPP_RELAUNCHTIMER_DATA();
					SlcApp.Trigger.UploadData = false;
				}

				// to upload dim
				if(SlcApp.Trigger.UploadDim)
				{
					APP_DEBUG("to upload dim");
					// SlcApp_Upload_LightDim();
					SlcApp.Trigger.UploadDim = false;
				}

				// upload custom at command
				if(SlcApp.Trigger.UploadCustomAtCmd)
				{
					APP_DEBUG("to upload custom at command");
					SlcApp_Upload_CustomAtCmd();
					SlcApp.Trigger.UploadCustomAtCmd = false;
				}

				if(SlcApp.Trigger.TerminateUpload)
				{
					APP_DEBUG("jump to SLCAPP_APPSTATE_TO_PELION_DEREG");
					appstate = SLCAPP_APPSTATE_TO_PELION_DEREG;
					SlcApp.Trigger.TerminateUpload = false;
				}

				#ifdef _NO_DISCONNECT_WHEN_FW_UPDATING
				if(SlcApp.Trigger.FWUpdate)
				{
					APP_DEBUG("stop timer and wait for fw update complete");
					SLCAPP_STOPTIMER_TERMINATEUPLOAD();
					SlcApp.FWUpdating = true;
					SlcApp.Trigger.FWUpdate = false;
				}
				
				if(SlcApp.Trigger.FWUpdateTerminated)
				{
					APP_DEBUG("fw update terminated and relaunch timer");
					SLCAPP_RELAUNCHTIMER_TERMINATEUPLOAD();
					SlcApp.FWUpdating = false;
					SlcApp.Trigger.FWUpdateTerminated = false;
				}
				#endif

				// to upload mcu mode (paired mode or sensor mode)
				if(SlcApp.McuMode != SlcCtrl_GetMcuMode())
				{
					_DateTimeStruct now;
					RTC_GetRealTime(&now);
					APP_DEBUG("to upload mcu mode to %d @ %d-%02d-%02d (w:%d) %02d:%02d:%02d", SlcCtrl_GetMcuMode(), now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
					// SlcApp_Upload_McuMode();
				}

				wait(1);
				break;

			case SLCAPP_APPSTATE_TO_PELION_DEREG:
				SLCAPP_STOPTIMER_NODEREGISTER();
				Pelion_ToDeregister(SlcAppConfig.config.deregister.bits.disconnect_type);
				SLCAPP_LED_DISCONNECTING();
				appstate = SLCAPP_APPSTATE_WAIT_PELION_DEREG;
				break;
				
			case SLCAPP_APPSTATE_WAIT_PELION_DEREG:
				if(!Pelion_IsRegistered() || Pelion_IsPaused())
				{
					disconnect_waitcnt = 0;
					appstate = SLCAPP_APPSTATE_WAIT_NETWORK_DISCONNECTED;
					APP_DEBUG("jump to SLCAPP_APPSTATE_WAIT_NETWORK_DISCONNECTED");
				}
				else
				{
					APP_DEBUG("waiting pelion degegistered...");
					//Main_PrintHeapStack(TAG);
					wait(DEBUG_DISPLAY_PERIOD_SEC);
				}
				break;
				
			case SLCAPP_APPSTATE_WAIT_NETWORK_DISCONNECTED:
				#ifdef _TO_CLOSE_NETWORK
				{
					_NetState netstat;
					netstat = Network_GetState();
					switch(netstat)
					{
						case NETSTATE_CONNECTING:
						case NETSTATE_CONNECTED:
						case NETSTATE_ERROR:
						case NETSTATE_ERROR_REBOOT:
							APP_DEBUG("waiting network or rrc disconnected...%d", disconnect_waitcnt+1);
							disconnect_waitcnt++;
							if(disconnect_waitcnt > (WAIT_DISCONNECT_TIMEOUT_SEC/DEBUG_DISPLAY_PERIOD_SEC))
							{
								APP_DEBUG("waiting for too long, reboot");
								OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
							}
							wait(DEBUG_DISPLAY_PERIOD_SEC);
							break;
						case NETSTATE_DISCONNECTED:
						case NETSTATE_RAIDISCONNECTED:
						case NETSTATE_DETACHDISCONNECTED:
							appstate = SLCAPP_APPSTATE_READY_TO_IDLE;
							APP_DEBUG("jump to SLCAPP_APPSTATE_READY_TO_IDLE");
							break;
						default:
							appstate = SLCAPP_APPSTATE_READY_TO_IDLE;
							APP_DEBUG("state error, jump to SLCAPP_APPSTATE_READY_TO_IDLE and reboot");
							OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
							break;
					}
				}
				#else
				appstate = SLCAPP_APPSTATE_READY_TO_IDLE;
				APP_DEBUG("jump to SLCAPP_APPSTATE_READY_TO_IDLE");				
				#endif
				break;
			
			case SLCAPP_APPSTATE_RESET:
				break;
		}

		//-------------------------------------------------------//
		// process light schedule
		//-------------------------------------------------------//
		for(i=0; i<LIGHT_SCHEDULE_COUNT_MAX; i++)
		{
			if(SlcAppLightScheduleGroup.light_schedule[i].enable)
			{
				if( SlcApp_DateTimeMatch(&(SlcAppLightScheduleGroup.light_schedule[i].datetime), NULL) )
				{
					APP_DEBUG("to update dim %d by light schedule[%d]", SlcAppLightScheduleGroup.light_schedule[i].dim, i);
					SlcAppLightDim.config.broadcast = 1;
					memset(SlcAppLightDim.config.dimarray, SlcAppLightScheduleGroup.light_schedule[i].dim, sizeof(SlcAppLightDim.config.dimarray));
					SlcCtrl_SetDimLevel(SlcAppLightDim.config.broadcast, SlcAppLightDim.config.dimarray, sizeof(SlcAppLightDim.config.dimarray));
					SlcApp.Trigger.UploadDim = true;
					SlcAppLightDimBackup.config.broadcast = SlcAppLightDim.config.broadcast;
					memcpy(SlcAppLightDimBackup.config.dimarray, SlcAppLightDim.config.dimarray, sizeof(SlcAppLightDim.config.dimarray));

					if(SlcAppLightScheduleGroup.light_schedule[i].repeat == 0)
					{
						APP_DEBUG("disable schedule[%d]", i);
						SlcAppLightScheduleGroup.light_schedule[i].enable = 0;
						SlcAppLightScheduleGroup.light_schedule[i].dim = 0;
						memset(SlcAppLightScheduleGroup.light_schedule[i].datetime.buff, 0, sizeof(SlcAppLightScheduleGroup.light_schedule[i].datetime.buff));
					}
					else if(SlcAppLightScheduleGroup.light_schedule[i].repeat != 0xFF)
					{
						SlcAppLightScheduleGroup.light_schedule[i].repeat--;
						APP_DEBUG("schedule[%d] repate %d", i, SlcAppLightScheduleGroup.light_schedule[i].repeat);
					}
					wait(UPDATE_LIGHT_SCHEDULE_PERIOD_SEC);
				}
			}
		}

		//-------------------------------------------------------//
		// check threshold
		//-------------------------------------------------------//
		// check voltage threshold
		if(SLCAPP_CHECKOUTOFSPEC_VOLTAGE())
		{
			if(!SlcAppStatus.bits.ErrorVoltage)
			{
				APP_DEBUG("ErrorVoltage 1");
				SlcAppStatus.bits.ErrorVoltage = true;
			}
		}
		else
		{
			if(SlcAppStatus.bits.ErrorVoltage)
			{
				APP_DEBUG("ErrorVoltage 0");
				SlcAppStatus.bits.ErrorVoltage = false;
			}
		}
		// check current threshold
		if(SLCAPP_CHECKOUTOFSPEC_CURRENT())
		{		
			if(!SlcAppStatus.bits.ErrorCurrent)
			{
				APP_DEBUG("ErrorCurrent 1");
				SlcAppStatus.bits.ErrorCurrent = true;
			}
		}
		else
		{
			if(SlcAppStatus.bits.ErrorCurrent)
			{
				APP_DEBUG("ErrorCurrent 0");
				SlcAppStatus.bits.ErrorCurrent = false;
			}
		}		
		// check freq threshold
		if(SLCAPP_CHECKOUTOFSPEC_FREQ())
		{
			if(!SlcAppStatus.bits.ErrorFreq)
			{
				APP_DEBUG("ErrorFreq 1");
				SlcAppStatus.bits.ErrorFreq = true;
			}
		}
		else
		{
			if(SlcAppStatus.bits.ErrorFreq)
			{
				APP_DEBUG("ErrorFreq 0");
				SlcAppStatus.bits.ErrorFreq = false;
			}
		}
		// check pf threshold
		if(SLCAPP_CHECKOUTOFSPEC_PF())
		{
			if(!SlcAppStatus.bits.ErrorPf)
			{
				APP_DEBUG("ErrorPf 1");
				SlcAppStatus.bits.ErrorPf = true;
			}
		}
		else
		{
			if(SlcAppStatus.bits.ErrorPf)
			{
				APP_DEBUG("ErrorPf 0");
				SlcAppStatus.bits.ErrorPf = false;
			}
		}
		// check watt threshold
		if(SLCAPP_CHECKOUTOFSPEC_WATT())
		{
			if(!SlcAppStatus.bits.ErrorWatt)
			{
				APP_DEBUG("ErrorWatt 1");
				SlcAppStatus.bits.ErrorWatt = true;
			}
		}
		else
		{
			if(SlcAppStatus.bits.ErrorWatt)
			{
				APP_DEBUG("ErrorWatt 0");
				SlcAppStatus.bits.ErrorWatt = false;
			}
		}

		//-------------------------------------------------------//
		// yield task
		//-------------------------------------------------------//
		SlcApp_KickThreadTimer();
		ThisThread::yield();
	}
}

void SlcApp_Init(void)
{
	APP_DEBUG("SlcApp Init");
	SlcApp_ResetStatus();

	DataStore_Init(SLCDATASTORE_DIR_NAME);
	SlcApp_RestoreConfig();
	SlcApp_RestoreLightSchedule();
	SlcApp_RestoreUploadTimeStamp();
	SlcApp_RestoreThreshold();
	
	SLCAPP_RELAUNCHTIMER_LIGHTSENSORMODE();
	SLCAPP_RELAUNCHTIMER_SYSTEMWATCHDOG();
	SLCAPP_RELAUNCHTIMER_THREADWATCHDOG();

	#if (THREAD_WATCHDOG_TIMEOUTTIME_SEC <= DEBUG_DISPLAY_PERIOD_LONG_SEC)
	#error "Thread watchdog timeout must larger than display sleep period"
	#endif												
	
	APP_DEBUG("SlcApp Launch");
	//SlcApp_Thread.start(SlcApp_ThreadFxn);
	SlcApp_Thread.start(callback(SlcApp_ThreadFxn, (void *)"THSlcApp"));
}


//-----------------------------------------------------------------------------------//
// action interface functions
//-----------------------------------------------------------------------------------//
void SlcApp_FWUpdateTriggered(void)
{
	#ifdef _NO_DISCONNECT_WHEN_FW_UPDATING
	APP_DEBUG("SlcApp_FWUpdateTriggered: trigger\n");
	SlcApp.Trigger.FWUpdate = true;
	#else
	APP_DEBUG("SlcApp_FWUpdateTriggered: do nothing\n");
	#endif
}

void SlcApp_FWUpdateTerminated(void)
{
	#ifdef _NO_DISCONNECT_WHEN_FW_UPDATING
	APP_DEBUG("SlcApp_FWUpdateTerminated: trigger\n");
	SlcApp.Trigger.FWUpdateTerminated = true;
	#else
	APP_DEBUG("SlcApp_FWUpdateTerminated: do nothing\n");
	#endif
}

#endif // #ifdef _PROJECT_SLC
