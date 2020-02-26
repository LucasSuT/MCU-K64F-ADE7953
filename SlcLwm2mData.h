#ifndef _PELION_DATA_FORMAT_SLC_H
#define _PELION_DATA_FORMAT_SLC_H


#include "simplem2mclient.h"

#define x_SIGNALPOWERLIGHT_RESOURCES	// define to create resources of data which contained in SignalPowerLight resource
										// undef to reduce amount of resources

// https://cloud.mbed.com/docs/current/connecting/device-guidelines.html
// To configure the cache lifetime, set the max-age parameter (in seconds) on the client (at the Resource level).
// The max-age is limited to 72 hours by the server side
#define SLC_RESOURCE_MAX_AGE	(5*60) //5mins //(36*3600)=36hr


#define SLC_RESOURCE_MAX_AT_CMD_STRING_SIZE		20
#define SLC_RESOURCE_MAX_AT_RSP_STRING_SIZE		300

//-----------------------------------------------------------------------------------//
// LwM2M resource defination
// <ObjectID>/<Instance>/<ResourceID>
// https://cloud.mbed.com/docs/v1.2/collecting/resource-model.html
// http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html
//-----------------------------------------------------------------------------------//
// ObjectID
// 32769-42768 This range is used for company bulk reservation (up to 50 ObjectIDs). This range is to create private objects that are not registered here.
// 32769-33254 used by some company
#define OBJECTID_AAEON					40000

// ResourceID
// 26241-32768 Private resource range, no registration is necessary and open to re-use.
#define RESOURCEID_CONFIG				30000
#define RESOURCEID_INFO					30100
#define RESOURCEID_POWER				30200
#define RESOURCEID_LIGHT				30300
#define RESOURCEID_ETC					30400
/*
enum {
	RESOURCEID_CONFIG_NODEREG			= RESOURCEID_CONFIG,
	RESOURCEID_CONFIG_CONFIG
};
enum {
	RESOURCEID_INFO_STATUS				= RESOURCEID_INFO,
	RESOURCEID_INFO_NODEVERSION,
	RESOURCEID_INFO_MCUVERSION,
	RESOURCEID_INFO_IMEI,
	RESOURCEID_INFO_IMSI,
	RESOURCEID_INFO_RSSI,
	RESOURCEID_INFO_MODFWVERSION,
	RESOURCEID_INFO_MODRADIOVERSION
};

enum {
	RESOURCEID_POWER_VOL				= RESOURCEID_POWER,
	RESOURCEID_POWER_FREQ,
	RESOURCEID_POWER_PF,
	RESOURCEID_POWER_CURRENT,
	RESOURCEID_POWER_WATT,
	RESOURCEID_POWER_TOTALWATT
};

enum {
	RESOURCEID_LIGHT_DIM				= RESOURCEID_LIGHT,
	RESOURCEID_LIGHT_LUX,
	RESOURCEID_LIGHT_LIGHTSENSORMODE,
	RESOURCEID_LIGHT_LIGHTSCHEDULE
};
*/

// slc resource index
enum {
	// config
	SLC_RESIDX_CONFIG_NODEREG,
	SLC_RESIDX_CONFIG_CONFIG,
	// SLC_RESIDX_CONFIG_TH_VOLTAGE_H,
	// SLC_RESIDX_CONFIG_TH_VOLTAGE_L,
	// SLC_RESIDX_CONFIG_TH_CURRENT_H,
	// SLC_RESIDX_CONFIG_TH_CURRENT_L,
	// SLC_RESIDX_CONFIG_TH_FREQ_H,
	// SLC_RESIDX_CONFIG_TH_FREQ_L,
	// SLC_RESIDX_CONFIG_TH_PF_H,
	// SLC_RESIDX_CONFIG_TH_PF_L,
	// SLC_RESIDX_CONFIG_TH_WATT_H,
	// SLC_RESIDX_CONFIG_TH_WATT_L,
	// SLC_RESIDX_CONFIG_TH_LUX_H,
	// SLC_RESIDX_CONFIG_TH_LUX_L,
	SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC,
	SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC,
	SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC,
	SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC,
	SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC,
	SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC,
	SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC,
	SLC_RESIDX_CONFIG_PERIOD_DATA_SEC,
	SLC_RESIDX_CONFIG_DEREG_KEEPONLINE,
	SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC,
		
	// info
	SLC_RESIDX_INFO_STATUS,
	SLC_RESIDX_INFO_NODEVERSION,
	SLC_RESIDX_INFO_MCUVERSION,
	SLC_RESIDX_INFO_IMEI,
	SLC_RESIDX_INFO_IMSI,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	SLC_RESIDX_INFO_RSSI,
	#endif
	SLC_RESIDX_INFO_MODFWVERSION,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	SLC_RESIDX_INFO_CELLID,
	SLC_RESIDX_INFO_RSRP,
	SLC_RESIDX_INFO_RSRQ,
	SLC_RESIDX_INFO_SNR,
	#endif
	SLC_RESIDX_INFO_CCID,

	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	// power
	SLC_RESIDX_POWER_VOL,
	SLC_RESIDX_POWER_FREQ,
	SLC_RESIDX_POWER_PF,
	SLC_RESIDX_POWER_CURRENT,
	SLC_RESIDX_POWER_WATT,
	SLC_RESIDX_POWER_TOTALWATT,
	#endif
	
	// light
	// SLC_RESIDX_LIGHT_DIM,
	// #ifdef _SIGNALPOWERLIGHT_RESOURCES
	// SLC_RESIDX_LIGHT_LUX,
	// SLC_RESIDX_LIGHT_MCUMODE,
	// #endif
	// SLC_RESIDX_LIGHT_LIGHTSCHEDULE,

	// etc
	SLC_RESIDX_ETC_SIGNALPOWERLIGHT,
	SLC_RESIDX_ETC_CUSTOMATCMD,
	
    SLC_RES_MAX
};
	
typedef enum _slc_resource_value_type {
    SLCRESVALUETYPE_STRING,
    SLCRESVALUETYPE_INT,
    SLCRESVALUETYPE_FLOAT,
    SLCRESVALUETYPE_BOOL
} _SlcResourceValueType;

typedef enum _slc_resource_operation_type {
	SLCRESOPTYPE_GET,
	SLCRESOPTYPE_PUT,
	SLCRESOPTYPE_POST,
	SLCRESOPTYPE_GETPUT,
	SLCRESOPTYPE_GETPOST,
	SLCRESOPTYPE_PUTPOST,
	SLCRESOPTYPE_GETPUTPOST
} _SlcResourceOperationType;
	
enum
{
	SLCAPP_DIM_ALLDIM,
	SLCAPP_DIM_BROADCAST,
	SLCAPP_DIM_SPECIFICDIM
};

//-----------------------------------------------------------------------------------//
// function declaration
//-----------------------------------------------------------------------------------//
void SlcLwm2mDate_CreateResource(SimpleM2MClient *mbedClient);

void SlcLwm2mDate_UploadNodeRegister(bool value);
void SlcLwm2mDate_UploadConfig(uint8_t *value, uint32_t value_length);
void SlcLwm2mDate_UploadStatus(int64_t value);
void SlcLwm2mDate_UploadThresholdVolatgeHigh(float value);
void SlcLwm2mDate_UploadThresholdVolatgeLow(float value);
void SlcLwm2mDate_UploadThresholdCurrentHigh(float value);
void SlcLwm2mDate_UploadThresholdCurrentLow(float value);
void SlcLwm2mDate_UploadThresholdFreqHigh(float value);
void SlcLwm2mDate_UploadThresholdFreqLow(float value);
void SlcLwm2mDate_UploadThresholdPfHigh(float value);
void SlcLwm2mDate_UploadThresholdPfLow(float value);
void SlcLwm2mDate_UploadThresholdWattHigh(float value);
void SlcLwm2mDate_UploadThresholdWattLow(float value);
void SlcLwm2mDate_UploadThresholdLuxHigh(int64_t value);
void SlcLwm2mDate_UploadThresholdLuxLow(int64_t value);
void SlcLwm2mDate_UploadConfigWatchdogSysSec(int64_t value);
void SlcLwm2mDate_UploadConfigWatchdogRegSec(int64_t value);
void SlcLwm2mDate_UploadConfigWatchdogLightSensorSec(int64_t value);
void SlcLwm2mDate_UploadConfigPeriodFirstUploadSec(int64_t value);
void SlcLwm2mDate_UploadConfigPeriodCycleUploadSec(int64_t value);
void SlcLwm2mDate_UploadConfigPeriodNodeRegisterSec(int64_t value);
void SlcLwm2mDate_UploadConfigPeriodStatusSec(int64_t value);
void SlcLwm2mDate_UploadConfigPeriodDataSec(int64_t value);
void SlcLwm2mDate_UploadConfigDeregKeepOnline(bool value);
void SlcLwm2mDate_UploadConfigDeregKeepOnlineTimeSec(int64_t value);
void SlcLwm2mDate_UploadNodeVersion(uint8_t *value, uint32_t value_length);
void SlcLwm2mDate_UploadMcuVersion(int64_t value);
void SlcLwm2mDate_UploadImei(uint8_t *value, uint32_t value_length);
void SlcLwm2mDate_UploadImsi(uint8_t *value, uint32_t value_length);
void SlcLwm2mDate_UploadRssi(int64_t value);
void SlcLwm2mDate_UploadModFwVer(uint8_t *value, uint32_t value_length);
void SlcLwm2mDate_UploadCellId(int64_t value);
void SlcLwm2mDate_UploadRsrp(int64_t value);
void SlcLwm2mDate_UploadRsrq(int64_t value);
void SlcLwm2mDate_UploadSnr(int64_t value);
void SlcLwm2mDate_UploadCcid(uint8_t *value, uint32_t value_length);
void SlcLwm2mDate_UploadPowerVol(float value);
void SlcLwm2mDate_UploadPowerFreq(float value);
void SlcLwm2mDate_UploadPowerPF(float value);
void SlcLwm2mDate_UploadPowerCurrent(float value);
void SlcLwm2mDate_UploadPowerWatt(float value);
void SlcLwm2mDate_UploadPowerTotalWatt(float value);
void SlcLwm2mDate_UploadLightDim(uint8_t *value, uint32_t value_length);
void SlcLwm2mDate_UploadLightLux(int64_t value);
void SlcLwm2mDate_UploadMcuMode(bool value);
void SlcLwm2mDate_UploadLightSchedule(uint8_t *value, uint32_t value_length);
void SlcLwm2mDate_UploadSignalPowerLight(uint8_t *value, uint32_t value_length);
void SlcLwm2mDate_UploadCustomAtCmd(uint8_t *value, uint32_t value_length);


void SlcLwm2mDate_ResetResourceSubscribed(void);
bool SlcLwm2mDate_IsAllResSubscribed(void);
bool *SlcLwm2mDate_ResSubscribedStatus(int *res_cnt);

#endif // #ifndef _PELION_DATA_FORMAT_SLC_H
	
