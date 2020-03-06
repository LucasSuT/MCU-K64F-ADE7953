#include "common_def.h"
#if defined(_PROJECT_SLC) || defined(_PROJECT_SPM)

#include "mbed.h"
#include "simplem2mclient.h"

#include "SlcLwm2mData.h"
#include "SlcControl.h"
#include "SlcApplication.h"
#include "CellularExternalControl.h"

#include "common_debug.h"
#ifdef _DEBUG_SLCLWM2MDATA
#define TAG						"LWM2MDATA"
#define LWM2MDATA_DEBUG			DEBUGMSG
#define LWM2MDATA_DUMP			DEBUGMSG_DUMP
#else
#define LWM2MDATA_DEBUG
#define LWM2MDATA_DUMP
#endif //_DEBUG_SCLLWM2MDATA

//-----------------------------------------------------------------------------------//
// function declaration
//-----------------------------------------------------------------------------------//
static void SlcRes_NodeRegister_PutCb(void *);
static void SlcRes_Config_PutCb(void *);
static void SlcRes_ThresholdVolatgeHigh_PutCb(void *);
static void SlcRes_ThresholdVolatgeLow_PutCb(void *);
static void SlcRes_ThresholdCurrentHigh_PutCb(void *);
static void SlcRes_ThresholdCurrentLow_PutCb(void *);
static void SlcRes_ThresholdFreqHigh_PutCb(void *);
static void SlcRes_ThresholdFreqLow_PutCb(void *);
static void SlcRes_ThresholdPfHigh_PutCb(void *);
static void SlcRes_ThresholdPfLow_PutCb(void *);
static void SlcRes_ThresholdWattHigh_PutCb(void *);
static void SlcRes_ThresholdWattLow_PutCb(void *);
static void SlcRes_ThresholdLuxHigh_PutCb(void *);
static void SlcRes_ThresholdLuxLow_PutCb(void *);
static void SlcRes_Config_WatchdogSysSec_PutCb(void *);
static void SlcRes_Config_WatchdogRegSec_PutCb(void *);
static void SlcRes_Config_WatchdogLightSensorSec_PutCb(void *);
static void SlcRes_Config_PeriodFirstUploadSec_PutCb(void *);
static void SlcRes_Config_PeriodCycleUploadSec_PutCb(void *);
static void SlcRes_Config_PeriodNodeRegisterSec_PutCb(void *);
static void SlcRes_Config_PeriodStatusSec_PutCb(void *);
static void SlcRes_Config_PeriodDataSec_PutCb(void *);
static void SlcRes_Config_DeregKeepOnline_PutCb(void *);
static void SlcRes_Config_DeregKeepOnlineTimeSec_PutCb(void *);
static void SlcRes_LightDim_PutCb(void *);
static void SlcRes_LightSchedule_PutCb(void *);
static void SlcRes_CustomAtCmd_PutCb(void *);

static void SlcRes_NodeRegister_PostCb(void *);
static void SlcRes_Config_PostCb(void *);
static void SlcRes_ThresholdVolatgeHigh_PostCb(void *);
static void SlcRes_ThresholdVolatgeLow_PostCb(void *);
static void SlcRes_ThresholdCurrentHigh_PostCb(void *);
static void SlcRes_ThresholdCurrentLow_PostCb(void *);
static void SlcRes_ThresholdFreqHigh_PostCb(void *);
static void SlcRes_ThresholdFreqLow_PostCb(void *);
static void SlcRes_ThresholdPfHigh_PostCb(void *);
static void SlcRes_ThresholdPfLow_PostCb(void *);
static void SlcRes_ThresholdWattHigh_PostCb(void *);
static void SlcRes_ThresholdWattLow_PostCb(void *);
static void SlcRes_ThresholdLuxHigh_PostCb(void *);
static void SlcRes_ThresholdLuxLow_PostCb(void *);
static void SlcRes_Config_WatchdogSysSec_PostCb(void *);
static void SlcRes_Config_WatchdogRegSec_PostCb(void *);
static void SlcRes_Config_WatchdogLightSensorSec_PostCb(void *);
static void SlcRes_Config_PeriodFirstUploadSec_PostCb(void *);
static void SlcRes_Config_PeriodCycleUploadSec_PostCb(void *);
static void SlcRes_Config_PeriodNodeRegisterSec_PostCb(void *);
static void SlcRes_Config_PeriodStatusSec_PostCb(void *);
static void SlcRes_Config_PeriodDataSec_PostCb(void *);
static void SlcRes_Config_DeregKeepOnline_PostCb(void *);
static void SlcRes_Config_DeregKeepOnlineTimeSec_PostCb(void *);
static void SlcRes_Status_PostCb(void *);
static void SlcRes_InfoNodeVersion_PostCb(void *);
static void SlcRes_InfoMcuVersion_PostCb(void *);
static void SlcRes_InfoImei_PostCb(void *);
static void SlcRes_InfoImsi_PostCb(void *);
static void SlcRes_InfoRssi_PostCb(void *);
static void SlcRes_InfoModFwVersion_PostCb(void *);
static void SlcRes_InfoCellId_PostCb(void *);
static void SlcRes_InfoRsrp_PostCb(void *);
static void SlcRes_InfoRsrq_PostCb(void *);
static void SlcRes_InfoSnr_PostCb(void *);
static void SlcRes_InfoCcid_PostCb(void *);
static void SlcRes_PowerVol_PostCb(void *);
static void SlcRes_PowerFreq_PostCb(void *);
static void SlcRes_PowerPf_PostCb(void *);
static void SlcRes_PowerCurrent_PostCb(void *);
static void SlcRes_PowerWatt_PostCb(void *);
static void SlcRes_PowerTotalWatt_PostCb(void *);
static void SlcRes_LightDim_PostCb(void *);
static void SlcRes_LightLux_PostCb(void *);
static void SlcRes_McuMode_PostCb(void *);
static void SlcRes_LightSchedule_PostCb(void *);
static void SlcRes_SignalPowerLight_PostCb(void *);


static void SlcRes_status_callback(const M2MBase& object,
                            const M2MBase::MessageDeliveryStatus status,
                            const M2MBase::MessageType /*type*/);
static void SlcRes_NodeRegister_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_Config_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ThresholdVolatgeHigh_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ThresholdVolatgeLow_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ThresholdCurrentHigh_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ThresholdCurrentLow_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ThresholdFreqHigh_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ThresholdFreqLow_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ThresholdPfHigh_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ThresholdPfLow_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ThresholdWattHigh_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ThresholdWattLow_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ThresholdLuxHigh_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ConfigWatchdogSysSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ConfigWatchdogRegSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ConfigWatchdogLightSensorSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ConfigPeriodFirstUploadSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ConfigPeriodCycleUploadSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ConfigPeriodNodeRegisterSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ConfigPeriodStatusSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ConfigPeriodDataSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ConfigDeregKeepOnline_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ConfigDeregKeepOnlineTimeSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_ThresholdLuxLow_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_InfoStatus_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_InfoNodeVersion_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_InfoMcuVersion_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_InfoImei_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_InfoImsi_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_InfoRssi_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_InfoModFwVersion_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_InfoCellId_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_InfoRsrp_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_InfoRsrq_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_InfoSnr_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_InfoCcid_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_PowerVol_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_PowerFreq_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_PowerPf_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_PowerCurrent_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_PowerWatt_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_PowerTotalWatt_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_LightDim_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_LightLux_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_McuMode_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_LightSchedule_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_SignalPowerLight_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);
static void SlcRes_CustomAtCmd_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type);


//-----------------------------------------------------------------------------------//
// global variables
//-----------------------------------------------------------------------------------//
M2MResource *SlcResource[SLC_RES_MAX];

uint16_t SlcResoure_PathValue[SLC_RES_MAX][3] = {
	// config
	{OBJECTID_AAEON, 0, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_NODEREG,
	{OBJECTID_AAEON, 1, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_CONFIG,
	// {OBJECTID_AAEON, 2, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_TH_VOLTAGE_H,
	// {OBJECTID_AAEON, 3, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_TH_VOLTAGE_L,
	// {OBJECTID_AAEON, 4, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_TH_CURRENT_H,
	// {OBJECTID_AAEON, 5, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_TH_CURRENT_L,
	// {OBJECTID_AAEON, 6, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_TH_FREQ_H,
	// {OBJECTID_AAEON, 7, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_TH_FREQ_L,
	// {OBJECTID_AAEON, 8, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_TH_PF_H,
	// {OBJECTID_AAEON, 9, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_TH_PF_L,
	// {OBJECTID_AAEON, 10, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_TH_WATT_H,
	// {OBJECTID_AAEON, 11, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_TH_WATT_L,
	// {OBJECTID_AAEON, 12, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_TH_LUX_H,
	// {OBJECTID_AAEON, 13, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_TH_LUX_L,
	{OBJECTID_AAEON, 14, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC,
	{OBJECTID_AAEON, 15, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC,
	{OBJECTID_AAEON, 16, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC,
	{OBJECTID_AAEON, 17, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC,
	{OBJECTID_AAEON, 18, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC,
	{OBJECTID_AAEON, 19, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC,
	{OBJECTID_AAEON, 20, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC,
	{OBJECTID_AAEON, 21, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_PERIOD_DATA_SEC,
	{OBJECTID_AAEON, 22, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_DEREG_KEEPONLINE,
	{OBJECTID_AAEON, 23, RESOURCEID_CONFIG}, 	// SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC,
	
	// info
	{OBJECTID_AAEON, 0, RESOURCEID_INFO},		// SLC_RESIDX_INFO_STATUS,
	{OBJECTID_AAEON, 1, RESOURCEID_INFO},		// SLC_RESIDX_INFO_NODEVERSION,
	{OBJECTID_AAEON, 2, RESOURCEID_INFO}, 		// SLC_RESIDX_INFO_MCUVERSION,
	{OBJECTID_AAEON, 3, RESOURCEID_INFO},		// SLC_RESIDX_INFO_IMEI,
	{OBJECTID_AAEON, 4, RESOURCEID_INFO},		// SLC_RESIDX_INFO_IMSI,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	{OBJECTID_AAEON, 5, RESOURCEID_INFO},		// SLC_RESIDX_INFO_RSSI,
	#endif
	{OBJECTID_AAEON, 6, RESOURCEID_INFO},		// SLC_RESIDX_INFO_MODFWVERSION,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	{OBJECTID_AAEON, 7, RESOURCEID_INFO},		// SLC_RESIDX_INFO_CELLID,
	{OBJECTID_AAEON, 8, RESOURCEID_INFO},		// SLC_RESIDX_INFO_RSRP,
	{OBJECTID_AAEON, 9, RESOURCEID_INFO},		// SLC_RESIDX_INFO_RSRQ,
	{OBJECTID_AAEON, 10, RESOURCEID_INFO},		// SLC_RESIDX_INFO_SNR,
	#endif
	{OBJECTID_AAEON, 11, RESOURCEID_INFO},		// SLC_RESIDX_INFO_CCID,	

	#ifdef _SIGNALPOWERLIGHT_RESOURCES	
	// power
	// {OBJECTID_AAEON, 0, RESOURCEID_POWER},		// SLC_RESIDX_POWER_VOL,
	// {OBJECTID_AAEON, 1, RESOURCEID_POWER}, 		// SLC_RESIDX_POWER_FREQ,
	// {OBJECTID_AAEON, 2, RESOURCEID_POWER},		// SLC_RESIDX_POWER_PF,
	// {OBJECTID_AAEON, 3, RESOURCEID_POWER},		// SLC_RESIDX_POWER_CURRENT,
	// {OBJECTID_AAEON, 4, RESOURCEID_POWER}, 		// SLC_RESIDX_POWER_WATT,
	// {OBJECTID_AAEON, 5, RESOURCEID_POWER},		// SLC_RESIDX_POWER_TOTALWATT,
	#endif
	
	// light
	// {OBJECTID_AAEON, 0, RESOURCEID_LIGHT},		// SLC_RESIDX_LIGHT_DIM,
	// #ifdef _SIGNALPOWERLIGHT_RESOURCES
	// {OBJECTID_AAEON, 1, RESOURCEID_LIGHT},		// SLC_RESIDX_LIGHT_LUX,
	// {OBJECTID_AAEON, 2, RESOURCEID_LIGHT},		// SLC_RESIDX_LIGHT_MCUMODE,
	// #endif
	// {OBJECTID_AAEON, 3, RESOURCEID_LIGHT},		// SLC_RESIDX_LIGHT_LIGHTSCHEDULE,

	// etc
	{OBJECTID_AAEON, 0, RESOURCEID_ETC},		// SLC_RESIDX_ETC_SIGNALPOWERLIGHT,	
	{OBJECTID_AAEON, 1, RESOURCEID_ETC}			// SLC_RESIDX_ETC_CUSTOMATCMD,
};

const char *SlcResource_Name[SLC_RES_MAX] = {
	// config
	"NodeRegister",								// SLC_RESIDX_CONFIG_NODEREG,
	"SysConfig",								// SLC_RESIDX_CONFIG_CONFIG,
	// "ThresholdVolatgeHigh",						// SLC_RESIDX_CONFIG_TH_VOLTAGE_H,
	// "ThresholdVolatgeLow",						// SLC_RESIDX_CONFIG_TH_VOLTAGE_L,
	// "ThresholdCurrentHigh",						// SLC_RESIDX_CONFIG_TH_CURRENT_H,
	// "ThresholdCurrentLow",						// SLC_RESIDX_CONFIG_TH_CURRENT_L,
	// "ThresholdFreqHigh",						// SLC_RESIDX_CONFIG_TH_FREQ_H,
	// "ThresholdFreqLow",							// SLC_RESIDX_CONFIG_TH_FREQ_L,
	// "ThresholdPfHigh",							// SLC_RESIDX_CONFIG_TH_PF_H,
	// "ThresholdPfLow",							// SLC_RESIDX_CONFIG_TH_PF_L,
	// "ThresholdWattHigh",						// SLC_RESIDX_CONFIG_TH_WATT_H,
	// "ThresholdWattLow",							// SLC_RESIDX_CONFIG_TH_WATT_L,
	// "ThresholdLuxHigh",							// SLC_RESIDX_CONFIG_TH_LUX_H,
	// "ThresholdLuxLow",							// SLC_RESIDX_CONFIG_TH_LUX_L,
	"WatchdogSysSec",							// SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC,
	"WatchdogRegSec",							// SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC,
	"WatchdogLightSensorSec",					// SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC,
	"PeriodFirstUploadSec",						// SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC,
	"PeriodCycleUploadSec",						// SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC,
	"PeriodNodeRegisterSec",					// SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC,
	"PeriodStatusSec",							// SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC,
	"PeriodDataSec",							// SLC_RESIDX_CONFIG_PERIOD_DATA_SEC,
	"PeriodDeregKeepOnline",					// SLC_RESIDX_CONFIG_DEREG_KEEPONLINE,
	"PeriodDeregKeepOnlineTimeSec",				// SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC,
	
	// info
	"SysStatus",								// SLC_RESIDX_INFO_STATUS,
	"NodeVersion",								// SLC_RESIDX_INFO_NODEVERSION,
	"McuVersion",								// SLC_RESIDX_INFO_MCUVERSION,
	"IMEI",										// SLC_RESIDX_INFO_IMEI,
	"IMSI",										// SLC_RESIDX_INFO_IMSI,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	"RSSI",										// SLC_RESIDX_INFO_RSSI,
	#endif
	"CellularFwVersion",						// SLC_RESIDX_INFO_MODFWVERSION,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	"CellId",									// SLC_RESIDX_INFO_CELLID,
	"RSRP",										// SLC_RESIDX_INFO_RSRP,
	"RSRQ",										// SLC_RESIDX_INFO_RSRQ,
	"SNR",										// SLC_RESIDX_INFO_SNR,
	#endif
	"CCID",										// SLC_RESIDX_INFO_CCID,

	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	// power
	// "Voltage",									// SLC_RESIDX_POWER_VOL,
	// "Freq",										// SLC_RESIDX_POWER_FREQ,
	// "PF",										// SLC_RESIDX_POWER_PF,
	// "Current",									// SLC_RESIDX_POWER_CURRENT,
	// "Watt",										// SLC_RESIDX_POWER_WATT,
	// "TotalWatt",								// SLC_RESIDX_POWER_TOTALWATT,
	#endif
	
	// light
	// "Dim",										// SLC_RESIDX_LIGHT_DIM,
	// #ifdef _SIGNALPOWERLIGHT_RESOURCES
	// "Lux",										// SLC_RESIDX_LIGHT_LUX,
	// "McuMode",									// SLC_RESIDX_LIGHT_MCUMODE,
	// #endif
	// "LightSchedule",							// SLC_RESIDX_LIGHT_LIGHTSCHEDULE,

	// etc
	"SignalPowerLight",							// SLC_RESIDX_ETC_SIGNALPOWERLIGHT,
	"CustomAtCmd"								// SLC_RESIDX_ETC_CUSTOMATCMD,
};

_SlcResourceValueType SlcResource_Value[SLC_RES_MAX] = {
	// config
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_CONFIG_NODEREG,
	SLCRESVALUETYPE_STRING,						// SLC_RESIDX_CONFIG_CONFIG,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_CONFIG_TH_VOLTAGE_H,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_CONFIG_TH_VOLTAGE_L,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_CONFIG_TH_CURRENT_H,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_CONFIG_TH_CURRENT_L,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_CONFIG_TH_FREQ_H,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_CONFIG_TH_FREQ_L,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_CONFIG_TH_PF_H,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_CONFIG_TH_PF_L,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_CONFIG_TH_WATT_H,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_CONFIG_TH_WATT_L,
	// SLCRESVALUETYPE_INT,						// SLC_RESIDX_CONFIG_TH_LUX_H,
	// SLCRESVALUETYPE_INT,						// SLC_RESIDX_CONFIG_TH_LUX_L,
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC,
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC,
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC,
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC,
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC,
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC,
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC,
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_CONFIG_PERIOD_DATA_SEC,
	SLCRESVALUETYPE_BOOL,						// SLC_RESIDX_CONFIG_DEREG_KEEPONLINE,
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC,
	
	// info
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_INFO_STATUS,
	SLCRESVALUETYPE_STRING,						// SLC_RESIDX_INFO_NODEVERSION,
	SLCRESVALUETYPE_STRING,						// SLC_RESIDX_INFO_MCUVERSION,
	SLCRESVALUETYPE_STRING,						// SLC_RESIDX_INFO_IMEI,
	SLCRESVALUETYPE_STRING,						// SLC_RESIDX_INFO_IMSI,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_INFO_RSSI,
	#endif
	SLCRESVALUETYPE_STRING,						// SLC_RESIDX_INFO_MODFWVERSION,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_INFO_CELLID,
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_INFO_RSRP,
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_INFO_RSRQ,
	SLCRESVALUETYPE_INT,						// SLC_RESIDX_INFO_SNR,
	#endif
	SLCRESVALUETYPE_STRING,						// SLC_RESIDX_INFO_CCID

	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	// power
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_POWER_VOL,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_POWER_FREQ,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_POWER_PF,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_POWER_CURRENT,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_POWER_WATT,
	// SLCRESVALUETYPE_FLOAT,						// SLC_RESIDX_POWER_TOTALWATT,
	#endif
	
	// light
	// SLCRESVALUETYPE_STRING,						// SLC_RESIDX_LIGHT_DIM,
	// #ifdef _SIGNALPOWERLIGHT_RESOURCES
	// SLCRESVALUETYPE_INT,						// SLC_RESIDX_LIGHT_LUX,
	// SLCRESVALUETYPE_BOOL,						// SLC_RESIDX_LIGHT_MCUMODE,
	// #endif
	// SLCRESVALUETYPE_STRING,						// SLC_RESIDX_LIGHT_LIGHTSCHEDULE,

	// etc
	SLCRESVALUETYPE_STRING,						// SLC_RESIDX_ETC_SIGNALPOWERLIGHT,
	SLCRESVALUETYPE_STRING,						// SLC_RESIDX_ETC_CUSTOMATCMD,
};
	
_SlcResourceOperationType SlcResource_Operation[SLC_RES_MAX] = {
	// config
	SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_NODEREG,
	SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_CONFIG,
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_TH_VOLTAGE_H,
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_TH_VOLTAGE_L,
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_TH_CURRENT_H,
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_TH_CURRENT_L,
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_TH_FREQ_H,
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_TH_FREQ_L,
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_TH_PF_H,
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_TH_PF_L,
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_TH_WATT_H,
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_TH_WATT_L,
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_TH_LUX_H,
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_TH_LUX_L,
	SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC,
	SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC,
	SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC,
	SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC,
	SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC,
	SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC,
	SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC,
	SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_PERIOD_DATA_SEC,
	SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_DEREG_KEEPONLINE,
	SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC,
	
	// info
	SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_INFO_STATUS,
	SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_INFO_NODEVERSION,
	SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_INFO_MCUVERSION,
	SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_INFO_IMEI,
	SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_INFO_IMSI,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_INFO_RSSI,
	#endif
	SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_INFO_MODFWVERSION,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_INFO_CELLID,
	SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_INFO_RSRP,
	SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_INFO_RSRQ,
	SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_INFO_SNR,
	#endif
	SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_INFO_CCID,

	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	// power
	// SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_POWER_VOL,
	// SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_POWER_FREQ,
	// SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_POWER_PF,
	// SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_POWER_CURRENT,
	// SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_POWER_WATT,
	// SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_POWER_TOTALWATT,
	#endif
	
	// light
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_LIGHT_DIM,
	// #ifdef _SIGNALPOWERLIGHT_RESOURCES
	// SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_LIGHT_LUX,
	// SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_LIGHT_MCUMODE,
	// #endif
	// SLCRESOPTYPE_GETPUTPOST, 					// SLC_RESIDX_LIGHT_LIGHTSCHEDULE,

	SLCRESOPTYPE_GETPOST, 						// SLC_RESIDX_ETC_SIGNALPOWERLIGHT,
	SLCRESOPTYPE_GETPUT		 					// SLC_RESIDX_ETC_CUSTOMATCMD,
};

_VoidVoidCb SlcResource_PutCb[SLC_RES_MAX] = {
	// config
	SlcRes_NodeRegister_PutCb, 					// SLC_RESIDX_CONFIG_NODEREG,
	SlcRes_Config_PutCb, 						// SLC_RESIDX_CONFIG_CONFIG,
	// SlcRes_ThresholdVolatgeHigh_PutCb,			// SLC_RESIDX_CONFIG_TH_VOLTAGE_H,
	// SlcRes_ThresholdVolatgeLow_PutCb,			// SLC_RESIDX_CONFIG_TH_VOLTAGE_L,
	// SlcRes_ThresholdCurrentHigh_PutCb,			// SLC_RESIDX_CONFIG_TH_CURRENT_H,
	// SlcRes_ThresholdCurrentLow_PutCb,			// SLC_RESIDX_CONFIG_TH_CURRENT_L,
	// SlcRes_ThresholdFreqHigh_PutCb,				// SLC_RESIDX_CONFIG_TH_FREQ_H,
	// SlcRes_ThresholdFreqLow_PutCb,				// SLC_RESIDX_CONFIG_TH_FREQ_L,
	// SlcRes_ThresholdPfHigh_PutCb,				// SLC_RESIDX_CONFIG_TH_PF_H,
	// SlcRes_ThresholdPfLow_PutCb,				// SLC_RESIDX_CONFIG_TH_PF_L,
	// SlcRes_ThresholdWattHigh_PutCb,				// SLC_RESIDX_CONFIG_TH_WATT_H,
	// SlcRes_ThresholdWattLow_PutCb,				// SLC_RESIDX_CONFIG_TH_WATT_L,
	// SlcRes_ThresholdLuxHigh_PutCb,				// SLC_RESIDX_CONFIG_TH_LUX_H,
	// SlcRes_ThresholdLuxLow_PutCb,				// SLC_RESIDX_CONFIG_TH_LUX_L,
	SlcRes_Config_WatchdogSysSec_PutCb,			//SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC,
	SlcRes_Config_WatchdogRegSec_PutCb,			//SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC,
	SlcRes_Config_WatchdogLightSensorSec_PutCb,	//SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC,
	SlcRes_Config_PeriodFirstUploadSec_PutCb,	//SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC,
	SlcRes_Config_PeriodCycleUploadSec_PutCb,	//SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC,
	SlcRes_Config_PeriodNodeRegisterSec_PutCb,	//SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC,
	SlcRes_Config_PeriodStatusSec_PutCb,		//SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC,
	SlcRes_Config_PeriodDataSec_PutCb,			//SLC_RESIDX_CONFIG_PERIOD_DATA_SEC,
	SlcRes_Config_DeregKeepOnline_PutCb,		//SLC_RESIDX_CONFIG_DEREG_KEEPONLINE,
	SlcRes_Config_DeregKeepOnlineTimeSec_PutCb,	//SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC,

	// info
	NULL,										// SLC_RESIDX_INFO_STATUS,
	NULL, 										// SLC_RESIDX_INFO_NODEVERSION,
	NULL, 										// SLC_RESIDX_INFO_MCUVERSION,
	NULL, 										// SLC_RESIDX_INFO_IMEI,
	NULL, 										// SLC_RESIDX_INFO_IMSI,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	NULL,										// SLC_RESIDX_INFO_RSSI,
	#endif
	NULL, 										// SLC_RESIDX_INFO_MODFWVERSION,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	NULL, 										// SLC_RESIDX_INFO_CELLID,
	NULL, 										// SLC_RESIDX_INFO_RSRP,
	NULL, 										// SLC_RESIDX_INFO_RSRQ,
	NULL, 										// SLC_RESIDX_INFO_SNR,
	#endif
	NULL, 										// SLC_RESIDX_INFO_CCID,

	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	// power
	// NULL,										// SLC_RESIDX_POWER_VOL,
	// NULL,										// SLC_RESIDX_POWER_FREQ,
	// NULL,										// SLC_RESIDX_POWER_PF,
	// NULL,										// SLC_RESIDX_POWER_CURRENT,
	// NULL,										// SLC_RESIDX_POWER_WATT,
	// NULL,										// SLC_RESIDX_POWER_TOTALWATT,
	#endif
	
	// light
	// SlcRes_LightDim_PutCb, 						// SLC_RESIDX_LIGHT_DIM,
	// #ifdef _SIGNALPOWERLIGHT_RESOURCES
	// NULL,										// SLC_RESIDX_LIGHT_LUX,
	// NULL,										// SLC_RESIDX_LIGHT_MCUMODE,
	// #endif
	// SlcRes_LightSchedule_PutCb,					// SLC_RESIDX_LIGHT_LIGHTSCHEDULE,

	// etc
	NULL, 										// SLC_RESIDX_ETC_SIGNALPOWERLIGHT,
	SlcRes_CustomAtCmd_PutCb					// SLC_RESIDX_ETC_CUSTOMATCMD,
};

_VoidVoidCb SlcResource_PostCb[SLC_RES_MAX] = {
	// config
	SlcRes_NodeRegister_PostCb, 				// SLC_RESIDX_CONFIG_NODEREG,
	SlcRes_Config_PostCb, 						// SLC_RESIDX_CONFIG_CONFIG,
	// SlcRes_ThresholdVolatgeHigh_PostCb,			// SLC_RESIDX_CONFIG_TH_VOLTAGE_H,
	// SlcRes_ThresholdVolatgeLow_PostCb,			// SLC_RESIDX_CONFIG_TH_VOLTAGE_L,
	// SlcRes_ThresholdCurrentHigh_PostCb,			// SLC_RESIDX_CONFIG_TH_CURRENT_H,
	// SlcRes_ThresholdCurrentLow_PostCb,			// SLC_RESIDX_CONFIG_TH_CURRENT_L,
	// SlcRes_ThresholdFreqHigh_PostCb,			// SLC_RESIDX_CONFIG_TH_FREQ_H,
	// SlcRes_ThresholdFreqLow_PostCb,				// SLC_RESIDX_CONFIG_TH_FREQ_L,
	// SlcRes_ThresholdPfHigh_PostCb,				// SLC_RESIDX_CONFIG_TH_PF_H,
	// SlcRes_ThresholdPfLow_PostCb,				// SLC_RESIDX_CONFIG_TH_PF_L,
	// SlcRes_ThresholdWattHigh_PostCb,			// SLC_RESIDX_CONFIG_TH_WATT_H,
	// SlcRes_ThresholdWattLow_PostCb,				// SLC_RESIDX_CONFIG_TH_WATT_L,
	// SlcRes_ThresholdLuxHigh_PostCb,				// SLC_RESIDX_CONFIG_TH_LUX_H,
	// SlcRes_ThresholdLuxLow_PostCb,				// SLC_RESIDX_CONFIG_TH_LUX_L,
	SlcRes_Config_WatchdogSysSec_PostCb,		// SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC,
	SlcRes_Config_WatchdogRegSec_PostCb,		// SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC,
	SlcRes_Config_WatchdogLightSensorSec_PostCb,// SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC,
	SlcRes_Config_PeriodFirstUploadSec_PostCb,	// SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC,
	SlcRes_Config_PeriodCycleUploadSec_PostCb,	// SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC,
	SlcRes_Config_PeriodNodeRegisterSec_PostCb,	// SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC,
	SlcRes_Config_PeriodStatusSec_PostCb,		// SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC,
	SlcRes_Config_PeriodDataSec_PostCb,			// SLC_RESIDX_CONFIG_PERIOD_DATA_SEC,
	SlcRes_Config_DeregKeepOnline_PostCb,		// SLC_RESIDX_CONFIG_DEREG_KEEPONLINE,
	SlcRes_Config_DeregKeepOnlineTimeSec_PostCb,// SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC,

	// info
	SlcRes_Status_PostCb,						// SLC_RESIDX_INFO_STATUS,
	SlcRes_InfoNodeVersion_PostCb, 				// SLC_RESIDX_INFO_NODEVERSION,
	SlcRes_InfoMcuVersion_PostCb, 				// SLC_RESIDX_INFO_MCUVERSION,
	SlcRes_InfoImei_PostCb, 					// SLC_RESIDX_INFO_IMEI,
	SlcRes_InfoImsi_PostCb, 					// SLC_RESIDX_INFO_IMSI,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	SlcRes_InfoRssi_PostCb,						// SLC_RESIDX_INFO_RSSI,
	#endif
	SlcRes_InfoModFwVersion_PostCb, 			// SLC_RESIDX_INFO_MODFWVERSION,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	SlcRes_InfoCellId_PostCb,					// SLC_RESIDX_INFO_CELLID,
	SlcRes_InfoRsrp_PostCb,						// SLC_RESIDX_INFO_RSRP,
	SlcRes_InfoRsrq_PostCb,						// SLC_RESIDX_INFO_RSRQ,
	SlcRes_InfoSnr_PostCb,						// SLC_RESIDX_INFO_SNR,
	#endif
	SlcRes_InfoCcid_PostCb,						// SLC_RESIDX_INFO_CCID,

	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	// power
	// SlcRes_PowerVol_PostCb,						// SLC_RESIDX_POWER_VOL,
	// SlcRes_PowerFreq_PostCb,					// SLC_RESIDX_POWER_FREQ,
	// SlcRes_PowerPf_PostCb,						// SLC_RESIDX_POWER_PF,
	// SlcRes_PowerCurrent_PostCb,					// SLC_RESIDX_POWER_CURRENT,
	// SlcRes_PowerWatt_PostCb,					// SLC_RESIDX_POWER_WATT,
	// SlcRes_PowerTotalWatt_PostCb,				// SLC_RESIDX_POWER_TOTALWATT,
	#endif
	
	// light
	// SlcRes_LightDim_PostCb, 					// SLC_RESIDX_LIGHT_DIM,
	// #ifdef _SIGNALPOWERLIGHT_RESOURCES
	// SlcRes_LightLux_PostCb,						// SLC_RESIDX_LIGHT_LUX,
	// SlcRes_McuMode_PostCb,						// SLC_RESIDX_LIGHT_MCUMODE,
	// #endif
	// SlcRes_LightSchedule_PostCb,				// SLC_RESIDX_LIGHT_LIGHTSCHEDULE,

	// etc
	SlcRes_SignalPowerLight_PostCb,				// SLC_RESIDX_ETC_SIGNALPOWERLIGHT,
	NULL										// SLC_RESIDX_ETC_CUSTOMATCMD,
};

uint32_t SlcResource_MaxAge[SLC_RES_MAX] = {
	// config
	SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_CONFIG_NODEREG,
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_CONFIG,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_TH_VOLTAGE_H,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_TH_VOLTAGE_L,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_TH_CURRENT_H,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_TH_CURRENT_L,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_TH_FREQ_H,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_TH_FREQ_L,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_TH_PF_H,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_TH_PF_L,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_TH_WATT_H,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_TH_WATT_L,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_TH_LUX_H,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_TH_LUX_L,
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC,
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC,
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC,
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC,
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC,
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC,
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC,
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_PERIOD_DATA_SEC,
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_DEREG_KEEPONLINE,
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC,
	
	// info
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_INFO_STATUS,
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_INFO_NODEVERSION,
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_INFO_MCUVERSION,
	SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_INFO_IMEI,
	SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_INFO_IMSI,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_INFO_RSSI,
	#endif
	SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_INFO_MODFWVERSION,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_INFO_CELLID,
	SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_INFO_RSRP,
	SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_INFO_RSRQ,
	SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_INFO_SNR,
	#endif
	SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_INFO_CCID,

	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	// power
	// SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_POWER_VOL,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_POWER_FREQ,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_POWER_PF,
	// SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_POWER_CURRENT,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_POWER_WATT,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_POWER_TOTALWATT,
	#endif
	
	// light
	// SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_LIGHT_DIM,
	// #ifdef _SIGNALPOWERLIGHT_RESOURCES
	// SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_LIGHT_LUX,
	// SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_LIGHT_MCUMODE,
	// #endif
	// SLC_RESOURCE_MAX_AGE, 						// SLC_RESIDX_LIGHT_LIGHTSCHEDULE,

	// etc
	SLC_RESOURCE_MAX_AGE,						// SLC_RESIDX_ETC_SIGNALPOWERLIGHT,
	SLC_RESOURCE_MAX_AGE						// SLC_RESIDX_ETC_CUSTOMATCMD,
};

bool SlcResource_AutoObservable[SLC_RES_MAX] = {
	// config
	false,										// SLC_RESIDX_CONFIG_NODEREG,
	false,										// SLC_RESIDX_CONFIG_CONFIG,
	// false,										// SLC_RESIDX_CONFIG_TH_VOLTAGE_H,
	// false,										// SLC_RESIDX_CONFIG_TH_VOLTAGE_L,
	// false,										// SLC_RESIDX_CONFIG_TH_CURRENT_H,
	// false,										// SLC_RESIDX_CONFIG_TH_CURRENT_L,
	// false,										// SLC_RESIDX_CONFIG_TH_FREQ_H,
	// false,										// SLC_RESIDX_CONFIG_TH_FREQ_L,
	// false,										// SLC_RESIDX_CONFIG_TH_PF_H,
	// false,										// SLC_RESIDX_CONFIG_TH_PF_L,
	// false,										// SLC_RESIDX_CONFIG_TH_WATT_H,
	// false,										// SLC_RESIDX_CONFIG_TH_WATT_L,
	// false,										// SLC_RESIDX_CONFIG_TH_LUX_H,
	// false,										// SLC_RESIDX_CONFIG_TH_LUX_L,
	false,										// SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC,
	false,										// SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC,
	false,										// SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC,
	false,										// SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC,
	false,										// SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC,
	false,										// SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC,
	false,										// SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC,
	false,										// SLC_RESIDX_CONFIG_PERIOD_DATA_SEC,
	false,										// SLC_RESIDX_CONFIG_DEREG_KEEPONLINE,
	false,										// SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC,
	
	// info
	false,										// SLC_RESIDX_INFO_STATUS,
	false,										// SLC_RESIDX_INFO_NODEVERSION,
	false,										// SLC_RESIDX_INFO_MCUVERSION,
	false,										// SLC_RESIDX_INFO_IMEI,
	false,										// SLC_RESIDX_INFO_IMSI,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	false,										// SLC_RESIDX_INFO_RSSI,
	#endif
	false,										// SLC_RESIDX_INFO_MODFWVERSION,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	false,										// SLC_RESIDX_INFO_CELLID,
	false,										// SLC_RESIDX_INFO_RSRP,
	false,										// SLC_RESIDX_INFO_RSRQ,
	false,										// SLC_RESIDX_INFO_SNR,
	#endif
	false,										// SLC_RESIDX_INFO_CCID

	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	// power
	// false,										// SLC_RESIDX_POWER_VOL,
	// false,										// SLC_RESIDX_POWER_FREQ,
	// false,										// SLC_RESIDX_POWER_PF,
	// false,										// SLC_RESIDX_POWER_CURRENT,
	// false,										// SLC_RESIDX_POWER_WATT,
	// false,										// SLC_RESIDX_POWER_TOTALWATT,
	#endif
	
	// light
	// false,										// SLC_RESIDX_LIGHT_DIM,
	// #ifdef _SIGNALPOWERLIGHT_RESOURCES
	// false,										// SLC_RESIDX_LIGHT_LUX,
	// false,										// SLC_RESIDX_LIGHT_MCUMODE,
	// #endif
	// false,										// SLC_RESIDX_LIGHT_LIGHTSCHEDULE,

	// etc
	false,										// SLC_RESIDX_ETC_SIGNALPOWERLIGHT,
	false,										// SLC_RESIDX_ETC_CUSTOMATCMD,
};

typedef void (*VoidStatusCb)(const M2MBase&, M2MBase::MessageDeliveryStatus, M2MBase::MessageType);
VoidStatusCb SlcResource_StatusCb[SLC_RES_MAX] = {
	// config
	SlcRes_NodeRegister_StatusCb, 				// SLC_RESIDX_CONFIG_NODEREG,
	SlcRes_Config_StatusCb, 					// SLC_RESIDX_CONFIG_CONFIG,
	// SlcRes_ThresholdVolatgeHigh_StatusCb,		// SLC_RESIDX_CONFIG_TH_VOLTAGE_H,
	// SlcRes_ThresholdVolatgeLow_StatusCb,		// SLC_RESIDX_CONFIG_TH_VOLTAGE_L,
	// SlcRes_ThresholdCurrentHigh_StatusCb,		// SLC_RESIDX_CONFIG_TH_CURRENT_H,
	// SlcRes_ThresholdCurrentLow_StatusCb,		// SLC_RESIDX_CONFIG_TH_CURRENT_L,
	// SlcRes_ThresholdFreqHigh_StatusCb,			// SLC_RESIDX_CONFIG_TH_FREQ_H,
	// SlcRes_ThresholdFreqLow_StatusCb,			// SLC_RESIDX_CONFIG_TH_FREQ_L,
	// SlcRes_ThresholdPfHigh_StatusCb,			// SLC_RESIDX_CONFIG_TH_PF_H,
	// SlcRes_ThresholdPfLow_StatusCb,				// SLC_RESIDX_CONFIG_TH_PF_L,
	// SlcRes_ThresholdWattHigh_StatusCb,			// SLC_RESIDX_CONFIG_TH_WATT_H,
	// SlcRes_ThresholdWattLow_StatusCb,			// SLC_RESIDX_CONFIG_TH_WATT_L,
	// SlcRes_ThresholdLuxHigh_StatusCb,			// SLC_RESIDX_CONFIG_TH_LUX_H,
	// SlcRes_ThresholdLuxLow_StatusCb,			// SLC_RESIDX_CONFIG_TH_LUX_L,
	SlcRes_ConfigWatchdogSysSec_StatusCb,		// SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC,
	SlcRes_ConfigWatchdogRegSec_StatusCb,		// SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC,
	SlcRes_ConfigWatchdogLightSensorSec_StatusCb,// SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC,
	SlcRes_ConfigPeriodFirstUploadSec_StatusCb,	// SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC,
	SlcRes_ConfigPeriodCycleUploadSec_StatusCb,	// SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC,
	SlcRes_ConfigPeriodNodeRegisterSec_StatusCb,// SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC,
	SlcRes_ConfigPeriodStatusSec_StatusCb,		// SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC,
	SlcRes_ConfigPeriodDataSec_StatusCb,		// SLC_RESIDX_CONFIG_PERIOD_DATA_SEC,
	SlcRes_ConfigDeregKeepOnline_StatusCb,		// SLC_RESIDX_CONFIG_DEREG_KEEPONLINE,
	SlcRes_ConfigDeregKeepOnlineTimeSec_StatusCb,// SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC,

	// info
	SlcRes_InfoStatus_StatusCb,					// SLC_RESIDX_INFO_STATUS,
	SlcRes_InfoNodeVersion_StatusCb, 			// SLC_RESIDX_INFO_NODEVERSION,
	SlcRes_InfoMcuVersion_StatusCb, 			// SLC_RESIDX_INFO_MCUVERSION,
	SlcRes_InfoImei_StatusCb, 					// SLC_RESIDX_INFO_IMEI,
	SlcRes_InfoImsi_StatusCb, 					// SLC_RESIDX_INFO_IMSI,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	SlcRes_InfoRssi_StatusCb,					// SLC_RESIDX_INFO_RSSI,
	#endif
	SlcRes_InfoModFwVersion_StatusCb, 			// SLC_RESIDX_INFO_MODFWVERSION,
	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	SlcRes_InfoCellId_StatusCb,					// SLC_RESIDX_INFO_CELLID,
	SlcRes_InfoRsrp_StatusCb,					// SLC_RESIDX_INFO_RSRP,
	SlcRes_InfoRsrq_StatusCb,					// SLC_RESIDX_INFO_RSRQ,
	SlcRes_InfoSnr_StatusCb,					// SLC_RESIDX_INFO_SNR,
	#endif
	SlcRes_InfoCcid_StatusCb,					// SLC_RESIDX_INFO_CCID,

	#ifdef _SIGNALPOWERLIGHT_RESOURCES
	// power
	// SlcRes_PowerVol_StatusCb,					// SLC_RESIDX_POWER_VOL,
	// SlcRes_PowerFreq_StatusCb,					// SLC_RESIDX_POWER_FREQ,
	// SlcRes_PowerPf_StatusCb,					// SLC_RESIDX_POWER_PF,
	// SlcRes_PowerCurrent_StatusCb,				// SLC_RESIDX_POWER_CURRENT,
	// SlcRes_PowerWatt_StatusCb,					// SLC_RESIDX_POWER_WATT,
	// SlcRes_PowerTotalWatt_StatusCb,				// SLC_RESIDX_POWER_TOTALWATT,
	#endif
	
	// light
	// SlcRes_LightDim_StatusCb, 					// SLC_RESIDX_LIGHT_DIM,
	// #ifdef _SIGNALPOWERLIGHT_RESOURCES
	// SlcRes_LightLux_StatusCb,					// SLC_RESIDX_LIGHT_LUX,
	// SlcRes_McuMode_StatusCb,					// SLC_RESIDX_LIGHT_MCUMODE,
	// #endif
	// SlcRes_LightSchedule_StatusCb,				// SLC_RESIDX_LIGHT_LIGHTSCHEDULE,

	// etc
	SlcRes_SignalPowerLight_StatusCb,			// SLC_RESIDX_ETC_SIGNALPOWERLIGHT,
	SlcRes_CustomAtCmd_StatusCb					// SLC_RESIDX_ETC_CUSTOMATCMD,
};

bool SlcResource_Subscribed[SLC_RES_MAX];


//-----------------------------------------------------------------------------------//
// functions
//-----------------------------------------------------------------------------------//
void SlcLwm2mDate_CreateResource(SimpleM2MClient *mbedClient)
{
	int i;
	M2MResourceInstance::ResourceType data_type;
	M2MBase::Operation allowed;
	bool observable;
	
	for(i=0; i<SLC_RES_MAX; i++)
	{
		switch(SlcResource_Value[i])
		{
			case SLCRESVALUETYPE_STRING:
				data_type = M2MResourceInstance::STRING;
				break;
			case SLCRESVALUETYPE_INT:
				data_type = M2MResourceInstance::INTEGER;
				break;
			case SLCRESVALUETYPE_FLOAT:
				data_type = M2MResourceInstance::FLOAT;
				break;
			case SLCRESVALUETYPE_BOOL:
				data_type = M2MResourceInstance::BOOLEAN;
				break;
		}
	
		switch(SlcResource_Operation[i])
		{
			case SLCRESOPTYPE_GET:
				allowed = M2MBase::GET_ALLOWED;
				observable = true;
				break;
			case SLCRESOPTYPE_PUT:
				allowed = M2MBase::PUT_ALLOWED;
				observable = false;
				break;
			case SLCRESOPTYPE_POST:
				allowed = M2MBase::POST_ALLOWED;
				observable = false;
				break;
			case SLCRESOPTYPE_GETPUT:
				allowed = M2MBase::GET_PUT_ALLOWED;
				observable = true;
				break;
			case SLCRESOPTYPE_GETPOST:
				allowed = M2MBase::GET_POST_ALLOWED;
				observable = true;
				break;
			case SLCRESOPTYPE_PUTPOST:
				allowed = M2MBase::PUT_POST_ALLOWED;
				observable = true;
				break;
			case SLCRESOPTYPE_GETPUTPOST:
				allowed = M2MBase::GET_PUT_POST_ALLOWED;
				observable = true;
				break;
		}

		// create resource
		LWM2MDATA_DEBUG("create resource: %d %s", i, SlcResource_Name[i]);
		SlcResource[i] = mbedClient->add_cloud_resource(SlcResoure_PathValue[i][0], SlcResoure_PathValue[i][1], SlcResoure_PathValue[i][2],
												SlcResource_Name[i], data_type, allowed, NULL, observable, 
												(void*)SlcResource_PutCb[i], (void*)SlcResource_PostCb[i], (void *)SlcResource_StatusCb[i]);
		// config resource data cache lifetime
		if(SlcResource_MaxAge[i] > 0)
			SlcResource[i]->set_max_age((const uint32_t)SlcResource_MaxAge[i]);

		// config resource as auto observable if config as true
		if(SlcResource_AutoObservable[i])
		{
			SlcResource[i]->set_auto_observable(true);
			SlcResource_Subscribed[i] = true;
		}
		SlcResource_Subscribed[i] = true;
	}
}


//-----------------------------------------------------------------------------------------//
// set resource value interface
//-----------------------------------------------------------------------------------------//
static void SlcLwm2mDate_SetValueString(int idx, uint8_t *value, uint32_t value_length)
{
	SlcResource[idx]->set_value(value, value_length);
}
static void SlcLwm2mDate_SetValueRaw(int idx, uint8_t *value, uint32_t value_length)
{
	SlcResource[idx]->set_value_raw(value, (const uint32_t)value_length);
}
static void SlcLwm2mDate_SetValueInt64(int idx, int64_t value)
{
	SlcResource[idx]->set_value(value);
}
static void SlcLwm2mDate_SetValueFloat(int idx, float value)
{
	SlcResource[idx]->set_value_float(value);
}
static void SlcLwm2mDate_SetValueBool(int idx, bool value)
{
	SlcResource[idx]->set_value(value);
}

void SlcLwm2mDate_UploadNodeRegister(bool value)
{
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_CONFIG_NODEREG;
	RTC_PrintRealTime();
	printf("upload %s [bool] %d\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueBool(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadConfig(uint8_t *value, uint32_t value_length)
{
	//SlcLwm2mDate_SetValueRaw(SLC_RESIDX_CONFIG_CONFIG, value, value_length);
	
	uint32_t tosend, i;
	char buff[SCLAPP_CONFIGBUFF_LEN*2];
	char *ptr;
	memset(buff, 0, sizeof(buff));
	tosend = (value_length*2 > sizeof(buff))? sizeof(buff)/2 : value_length;
	if(value_length*2 > sizeof(buff))
	{
		printf("%s[%d] buff define not enough!!!\n", __func__, __LINE__);
	}
	ptr = buff;
	for(i=0; i<tosend; i++)
	{
		sprintf(ptr, "%02x", *value);
		ptr += 2;
		value++;
	}
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_CONFIG_CONFIG;
	RTC_PrintRealTime();
	printf("upload %s [%d]\n", SlcResource_Name[residx], (int)value_length);
	SlcLwm2mDate_SetValueString(residx, (uint8_t *)buff, tosend*2);
	//printf("---------------------------------\n");
}
// void SlcLwm2mDate_UploadThresholdVolatgeHigh(float value)
// { 
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_CONFIG_TH_VOLTAGE_H;
// 	RTC_PrintRealTime();
// 	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueFloat(residx, value);
// 	//printf("---------------------------------\n");
// }
// void SlcLwm2mDate_UploadThresholdVolatgeLow(float value)
// { 
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_CONFIG_TH_VOLTAGE_L;
// 	RTC_PrintRealTime();
// 	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueFloat(residx, value);
// 	//printf("---------------------------------\n");
// }
// void SlcLwm2mDate_UploadThresholdCurrentHigh(float value)
// { 
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_CONFIG_TH_CURRENT_H;
// 	RTC_PrintRealTime();
// 	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueFloat(residx, value);
// 	//printf("---------------------------------\n");
// }
// void SlcLwm2mDate_UploadThresholdCurrentLow(float value)
// { 
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_CONFIG_TH_CURRENT_L;
// 	RTC_PrintRealTime();
// 	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueFloat(residx, value);
// 	//printf("---------------------------------\n");
// }
// void SlcLwm2mDate_UploadThresholdFreqHigh(float value)
// { 
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_CONFIG_TH_FREQ_H;
// 	RTC_PrintRealTime();
// 	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueFloat(residx, value);
// 	//printf("---------------------------------\n");
// }
// void SlcLwm2mDate_UploadThresholdFreqLow(float value)
// { 
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_CONFIG_TH_FREQ_L;
// 	RTC_PrintRealTime();
// 	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueFloat(residx, value);
// 	//printf("---------------------------------\n");
// }
// void SlcLwm2mDate_UploadThresholdPfHigh(float value)
// { 
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_CONFIG_TH_PF_H;
// 	RTC_PrintRealTime();
// 	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueFloat(residx, value);
// 	//printf("---------------------------------\n");
// }
// void SlcLwm2mDate_UploadThresholdPfLow(float value)
// { 
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_CONFIG_TH_PF_L;
// 	RTC_PrintRealTime();
// 	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueFloat(residx, value);
// 	//printf("---------------------------------\n");
// }
// void SlcLwm2mDate_UploadThresholdWattHigh(float value)
// { 
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_CONFIG_TH_WATT_H;
// 	RTC_PrintRealTime();
// 	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueFloat(residx, value);
// 	//printf("---------------------------------\n");
// }
// void SlcLwm2mDate_UploadThresholdWattLow(float value)
// { 
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_CONFIG_TH_WATT_L;
// 	RTC_PrintRealTime();
// 	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueFloat(residx, value);
// 	//printf("---------------------------------\n");
// }
// void SlcLwm2mDate_UploadThresholdLuxHigh(int64_t value)
// { 
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_CONFIG_TH_LUX_H;
// 	RTC_PrintRealTime();
// 	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueInt64(residx, value);
// 	//printf("---------------------------------\n");
// }
// void SlcLwm2mDate_UploadThresholdLuxLow(int64_t value)
// { 
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_CONFIG_TH_LUX_L;
// 	RTC_PrintRealTime();
// 	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueInt64(residx, value);
// 	//printf("---------------------------------\n");
// }
void SlcLwm2mDate_UploadConfigWatchdogSysSec(int64_t value)
{ 
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadConfigWatchdogRegSec(int64_t value)
{ 
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadConfigWatchdogLightSensorSec(int64_t value)
{ 
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadConfigPeriodFirstUploadSec(int64_t value)
{ 
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadConfigPeriodCycleUploadSec(int64_t value)
{ 
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadConfigPeriodNodeRegisterSec(int64_t value)
{ 
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadConfigPeriodStatusSec(int64_t value)
{ 
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadConfigPeriodDataSec(int64_t value)
{ 
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_CONFIG_PERIOD_DATA_SEC;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadConfigDeregKeepOnline(bool value)
{ 
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_CONFIG_DEREG_KEEPONLINE;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadConfigDeregKeepOnlineTimeSec(int64_t value)
{ 
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadStatus(int64_t value)
{
	//wait(5);
	//printf("---------------------------------\n");
	RTC_PrintRealTime();
	printf("upload status [int64_t] %lld\n", value);
	SlcLwm2mDate_SetValueInt64(SLC_RESIDX_INFO_STATUS, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadNodeVersion(uint8_t *value, uint32_t value_length)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_INFO_NODEVERSION;
	RTC_PrintRealTime();
	printf("upload %s [%d] %s\n", SlcResource_Name[residx], (int)value_length, value);
	SlcLwm2mDate_SetValueString(residx, value, value_length);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadMcuVersion(int64_t value)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_INFO_MCUVERSION;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadImei(uint8_t *value, uint32_t value_length)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_INFO_IMEI;
	RTC_PrintRealTime();
	printf("upload %s [%d] %s\n", SlcResource_Name[residx], (int)value_length, value);
	SlcLwm2mDate_SetValueString(residx, value, value_length);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadImsi(uint8_t *value, uint32_t value_length)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_INFO_IMSI;
	RTC_PrintRealTime();
	printf("upload %s [%d] %s\n", SlcResource_Name[residx], (int)value_length, value);
	SlcLwm2mDate_SetValueString(residx, value, value_length);
	//printf("---------------------------------\n");
}
#ifdef _SIGNALPOWERLIGHT_RESOURCES
void SlcLwm2mDate_UploadRssi(int64_t value)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_INFO_RSSI;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
#else
void SlcLwm2mDate_UploadRssi(int64_t value) {}
#endif
void SlcLwm2mDate_UploadModFwVer(uint8_t *value, uint32_t value_length)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_INFO_MODFWVERSION;
	RTC_PrintRealTime();
	printf("upload %s [%d] %s\n", SlcResource_Name[residx], (int)value_length, value);
	SlcLwm2mDate_SetValueString(residx, value, value_length);
	//printf("---------------------------------\n");
}
#ifdef _SIGNALPOWERLIGHT_RESOURCES
void SlcLwm2mDate_UploadCellId(int64_t value)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_INFO_CELLID;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadRsrp(int64_t value)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_INFO_RSRP;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadRsrq(int64_t value)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_INFO_RSRQ;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadSnr(int64_t value)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_INFO_SNR;
	RTC_PrintRealTime();
	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueInt64(residx, value);
	//printf("---------------------------------\n");
}
#else
void SlcLwm2mDate_UploadCellId(int64_t value) {}
void SlcLwm2mDate_UploadRsrp(int64_t value) {}
void SlcLwm2mDate_UploadRsrq(int64_t value) {}
void SlcLwm2mDate_UploadSnr(int64_t value) {}
#endif
void SlcLwm2mDate_UploadCcid(uint8_t *value, uint32_t value_length)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_INFO_CCID;
	RTC_PrintRealTime();
	printf("upload %s [%d] %s\n", SlcResource_Name[residx], (int)value_length, value);
	SlcLwm2mDate_SetValueString(residx, value, value_length);
	//printf("---------------------------------\n");
}
#ifdef _SIGNALPOWERLIGHT_RESOURCES
void SlcLwm2mDate_UploadPowerVol(float value)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_POWER_VOL;
	RTC_PrintRealTime();
	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueFloat(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadPowerFreq(float value)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_POWER_FREQ;
	RTC_PrintRealTime();
	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueFloat(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadPowerPF(float value)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_POWER_PF;
	RTC_PrintRealTime();
	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueFloat(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadPowerCurrent(float value)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_POWER_CURRENT;
	RTC_PrintRealTime();
	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueFloat(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadPowerWatt(float value)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_POWER_WATT;
	RTC_PrintRealTime();
	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueFloat(residx, value);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadPowerTotalWatt(float value)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_POWER_TOTALWATT;
	RTC_PrintRealTime();
	printf("upload %s [float] %f\n", SlcResource_Name[residx], value);
	SlcLwm2mDate_SetValueFloat(residx, value);
}
#else
void SlcLwm2mDate_UploadPowerVol(float value) {}
void SlcLwm2mDate_UploadPowerFreq(float value) {}
void SlcLwm2mDate_UploadPowerPF(float value) {}
void SlcLwm2mDate_UploadPowerCurrent(float value) {}
void SlcLwm2mDate_UploadPowerWatt(float value) {}
void SlcLwm2mDate_UploadPowerTotalWatt(float value) {}
#endif
// void SlcLwm2mDate_UploadLightDim(uint8_t *value, uint32_t value_length)
// {
// 	//SlcLwm2mDate_SetValueRaw(SLC_RESIDX_LIGHT_DIM, value, value_length);
	
// 	uint32_t tosend, i;
// 	char buff[SLCAPP_LIGHTDIMSTRUCT_LEN*2];
// 	char *ptr;
// 	memset(buff, 0, sizeof(buff));
// 	tosend = (value_length*2 > sizeof(buff))? sizeof(buff)/2 : value_length;
// 	if(value_length*2 > sizeof(buff))
// 	{
// 		printf("%s[%d] buff define not enough!!!\n", __func__, __LINE__);
// 	}
// 	ptr = buff;
// 	for(i=0; i<tosend; i++)
// 	{
// 		sprintf(ptr, "%02x", *value);
// 		ptr += 2;
// 		value++;
// 	}
// 	//wait(5);
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_LIGHT_DIM;
// 	RTC_PrintRealTime();
// 	printf("upload %s [%d]\n", SlcResource_Name[residx], (int)value_length);
// 	SlcLwm2mDate_SetValueString(residx, (uint8_t *)buff, tosend*2);
// 	//printf("---------------------------------\n");
// }
#ifdef _SIGNALPOWERLIGHT_RESOURCES
// void SlcLwm2mDate_UploadLightLux(int64_t value)
// {
// 	//wait(5);
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_LIGHT_LUX;
// 	RTC_PrintRealTime();
// 	printf("upload %s [int64_t] %lld\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueInt64(residx, value);
// 	//printf("---------------------------------\n");
// }
// void SlcLwm2mDate_UploadMcuMode(bool value)
// {
// 	//wait(5);
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_LIGHT_MCUMODE;
// 	RTC_PrintRealTime();
// 	printf("upload %s [bool] %d\n", SlcResource_Name[residx], value);
// 	SlcLwm2mDate_SetValueBool(residx, value);
// 	//printf("---------------------------------\n");
// }
#else
// void SlcLwm2mDate_UploadLightLux(int64_t value) {}
// void SlcLwm2mDate_UploadMcuMode(bool value) {}
#endif
// void SlcLwm2mDate_UploadLightSchedule(uint8_t *value, uint32_t value_length)
// {
// 	//SlcLwm2mDate_SetValueRaw(SLC_RESIDX_LIGHT_LIGHTSCHEDULE, value, value_length);
	
// 	uint32_t tosend, i;
// 	char buff[SLCAPP_LIGHTSCHEDULEGROUP_LEN*2];
// 	char *ptr;
// 	memset(buff, 0, sizeof(buff));
// 	tosend = (value_length*2 > sizeof(buff))? sizeof(buff)/2 : value_length;
// 	if(value_length*2 > sizeof(buff))
// 	{
// 		printf("%s[%d] buff define not enough!!!\n", __func__, __LINE__);
// 		printf("value_length: %d, sizeof(buff): %d\n", (int)value_length, sizeof(buff));
// 	}
// 	ptr = buff;
// 	for(i=0; i<tosend; i++)
// 	{
// 		sprintf(ptr, "%02x", *value);
// 		ptr += 2;
// 		value++;
// 	}
// 	//wait(5);
// 	//printf("---------------------------------\n");
// 	int residx = SLC_RESIDX_LIGHT_LIGHTSCHEDULE;
// 	RTC_PrintRealTime();
// 	printf("upload %s [%d]\n", SlcResource_Name[residx], (int)value_length);
// 	SlcLwm2mDate_SetValueString(residx, (uint8_t *)buff, tosend*2);
// 	//printf("---------------------------------\n");
// }
void SlcLwm2mDate_UploadSignalPowerLight(uint8_t *value, uint32_t value_length)
{
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_ETC_SIGNALPOWERLIGHT;
	RTC_PrintRealTime();
	printf("upload %s [%d] %s\n", SlcResource_Name[residx], (int)value_length, value);
	SlcLwm2mDate_SetValueString(residx, value, value_length);
	//printf("---------------------------------\n");
}
void SlcLwm2mDate_UploadCustomAtCmd(uint8_t *value, uint32_t value_length)
{
	//wait(5);
	//printf("---------------------------------\n");
	int residx = SLC_RESIDX_ETC_CUSTOMATCMD;
	RTC_PrintRealTime();
	printf("upload %s [%d] %s\n", SlcResource_Name[residx], (int)value_length, value);
	SlcLwm2mDate_SetValueString(residx, value, value_length);
	//printf("---------------------------------\n");
}

//-----------------------------------------------------------------------------------------//
// resource put callback function
// to update value from pelion to system
//-----------------------------------------------------------------------------------------//
static void SlcRes_NodeRegister_PutCb(void *)
{
	int residx;

	residx = SLC_RESIDX_CONFIG_NODEREG;
	LWM2MDATA_DEBUG("%s PUT received: %d", SlcResource_Name[residx], SlcResource[residx]->get_value_int());

	SlcApp_Update_NodeRegister(SlcResource[residx]->get_value_int());	
	SlcApp_KickAppTimer();
}

static void SlcRes_Config_PutCb(void *)
{
	int residx;

	residx = SLC_RESIDX_CONFIG_CONFIG;
	LWM2MDATA_DEBUG("%s PUT received: %s", SlcResource_Name[residx], SlcResource[residx]->get_value_string().c_str());

	uint32_t strsize, i, totransfer;
	char strbuff[SCLAPP_CONFIGBUFF_LEN*2];
	uint8_t buff[SCLAPP_CONFIGBUFF_LEN];
	uint8_t hi ,lo;
	strsize = strlen(SlcResource[residx]->get_value_string().c_str());
	totransfer = (strsize > sizeof(buff)*2)? sizeof(buff) : strsize/2;
	if(strsize > sizeof(buff)*2)
	{
		printf("[SlcLwm2mDat] SlcRes_Config_PutCb: buff size not enough!!!\n");
	}
	memcpy(strbuff, SlcResource[residx]->get_value_string().c_str(), totransfer*2);
	//LWM2MDATA_DEBUG("strbuff[%d] %s", totransfer*2, strbuff);
	memset(buff, 0, sizeof(buff));
	for(i=0; i<totransfer; i++)
	{
		if( (strbuff[i*2] >= '0') && (strbuff[i*2] <= '9') )
			hi = strbuff[i*2] - '0';
		else if( (strbuff[i*2] >= 'A') && (strbuff[i*2] <= 'F') )
			hi = strbuff[i*2] - 'A' + 10;
		else if( (strbuff[i*2] >= 'a') && (strbuff[i*2] <= 'f') )
			hi = strbuff[i*2] - 'a' + 10;
		else
			hi = 0;

		if( (strbuff[i*2+1] >= '0') && (strbuff[i*2+1] <= '9') )
			lo = strbuff[i*2+1] - '0';
		else if( (strbuff[i*2+1] >= 'A') && (strbuff[i*2+1] <= 'F') )
			lo = strbuff[i*2+1] - 'A' + 10;
		else if( (strbuff[i*2+1] >= 'a') && (strbuff[i*2+1] <= 'f') )
			lo = strbuff[i*2+1] - 'a' + 10;		
		else
			lo = 0;

		buff[i] = (hi << 4) + lo;
		//LWM2MDATA_DEBUG("[%d] [%x][%x] -> %x %x -> %02x", i, strbuff[i*2], strbuff[i*2+1], hi, lo, buff[i]);
	}
	LWM2MDATA_DUMP("Config Buff Dump", i, totransfer, buff);
	SlcApp_Update_Config(buff, totransfer);

	SlcApp_KickAppTimer();
}
// static void SlcRes_ThresholdVolatgeHigh_PutCb(void *)
// {
// 	int residx;

// 	residx = SLC_RESIDX_CONFIG_TH_VOLTAGE_H;
// 	LWM2MDATA_DEBUG("%s PUT received: %f", SlcResource_Name[residx], SlcResource[residx]->get_value_float());

// 	SlcApp_Update_ThresholdVolatgeHigh(SlcResource[residx]->get_value_float());
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdVolatgeLow_PutCb(void *)
// {
// 	int residx;

// 	residx = SLC_RESIDX_CONFIG_TH_VOLTAGE_L;
// 	LWM2MDATA_DEBUG("%s PUT received: %f", SlcResource_Name[residx], SlcResource[residx]->get_value_float());
	
// 	SlcApp_Update_ThresholdVolatgeLow(SlcResource[residx]->get_value_float());
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdCurrentHigh_PutCb(void *)
// {
// 	int residx;

// 	residx = SLC_RESIDX_CONFIG_TH_CURRENT_H;
// 	LWM2MDATA_DEBUG("%s PUT received: %f", SlcResource_Name[residx], SlcResource[residx]->get_value_float());
	
// 	SlcApp_Update_ThresholdCurrentHigh(SlcResource[residx]->get_value_float());
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdCurrentLow_PutCb(void *)
// {
// 	int residx;

// 	residx = SLC_RESIDX_CONFIG_TH_CURRENT_L;
// 	LWM2MDATA_DEBUG("%s PUT received: %f", SlcResource_Name[residx], SlcResource[residx]->get_value_float());
	
// 	SlcApp_Update_ThresholdCurrentLow(SlcResource[residx]->get_value_float());
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdFreqHigh_PutCb(void *)
// {
// 	int residx;

// 	residx = SLC_RESIDX_CONFIG_TH_FREQ_H;
// 	LWM2MDATA_DEBUG("%s PUT received: %f", SlcResource_Name[residx], SlcResource[residx]->get_value_float());
	
// 	SlcApp_Update_ThresholdFreqHigh(SlcResource[residx]->get_value_float());
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdFreqLow_PutCb(void *)
// {
// 	int residx;

// 	residx = SLC_RESIDX_CONFIG_TH_FREQ_L;
// 	LWM2MDATA_DEBUG("%s PUT received: %f", SlcResource_Name[residx], SlcResource[residx]->get_value_float());
	
// 	SlcApp_Update_ThresholdFreqLow(SlcResource[residx]->get_value_float());
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdPfHigh_PutCb(void *)
// {
// 	int residx;

// 	residx = SLC_RESIDX_CONFIG_TH_PF_H;
// 	LWM2MDATA_DEBUG("%s PUT received: %f", SlcResource_Name[residx], SlcResource[residx]->get_value_float());
	
// 	SlcApp_Update_ThresholdPfHigh(SlcResource[residx]->get_value_float());
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdPfLow_PutCb(void *)
// {
// 	int residx;

// 	residx = SLC_RESIDX_CONFIG_TH_PF_L;
// 	LWM2MDATA_DEBUG("%s PUT received: %f", SlcResource_Name[residx], SlcResource[residx]->get_value_float());
	
// 	SlcApp_Update_ThresholdPfLow(SlcResource[residx]->get_value_float());
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdWattHigh_PutCb(void *)
// {
// 	int residx;

// 	residx = SLC_RESIDX_CONFIG_TH_WATT_H;
// 	LWM2MDATA_DEBUG("%s PUT received: %f", SlcResource_Name[residx], SlcResource[residx]->get_value_float());
	
// 	SlcApp_Update_ThresholdWattHigh(SlcResource[residx]->get_value_float());
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdWattLow_PutCb(void *)
// {
// 	int residx;

// 	residx = SLC_RESIDX_CONFIG_TH_WATT_L;
// 	LWM2MDATA_DEBUG("%s PUT received: %f", SlcResource_Name[residx], SlcResource[residx]->get_value_float());
	
// 	SlcApp_Update_ThresholdWattLow(SlcResource[residx]->get_value_float());
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdLuxHigh_PutCb(void *)
// {
// 	int residx;

// 	residx = SLC_RESIDX_CONFIG_TH_LUX_H;
// 	LWM2MDATA_DEBUG("%s PUT received: %d", SlcResource_Name[residx], SlcResource[residx]->get_value_int());
	
// 	SlcApp_Update_ThresholdLuxHigh(SlcResource[residx]->get_value_int());
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdLuxLow_PutCb(void *)
// {
// 	int residx;

// 	residx = SLC_RESIDX_CONFIG_TH_LUX_L;
// 	LWM2MDATA_DEBUG("%s PUT received: %d", SlcResource_Name[residx], SlcResource[residx]->get_value_int());
	
// 	SlcApp_Update_ThresholdLuxLow(SlcResource[residx]->get_value_int());
// 	SlcApp_KickAppTimer();
// }
static void SlcRes_Config_WatchdogSysSec_PutCb(void *)
{
	int residx;

	residx = SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC;
	LWM2MDATA_DEBUG("%s PUT received: %d", SlcResource_Name[residx], SlcResource[residx]->get_value_int());
	
	SlcApp_Update_Config_WatchdogSysSec(SlcResource[residx]->get_value_int());
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_WatchdogRegSec_PutCb(void *)
{
	int residx;

	residx = SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC;
	LWM2MDATA_DEBUG("%s PUT received: %d", SlcResource_Name[residx], SlcResource[residx]->get_value_int());
	
	SlcApp_Update_Config_WatchdogRegSec(SlcResource[residx]->get_value_int());
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_WatchdogLightSensorSec_PutCb(void *)
{
	int residx;

	residx = SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC;
	LWM2MDATA_DEBUG("%s PUT received: %d", SlcResource_Name[residx], SlcResource[residx]->get_value_int());
	
	SlcApp_Update_Config_WatchdogLightSensorSec(SlcResource[residx]->get_value_int());
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_PeriodFirstUploadSec_PutCb(void *)
{
	int residx;

	residx = SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC;
	LWM2MDATA_DEBUG("%s PUT received: %d", SlcResource_Name[residx], SlcResource[residx]->get_value_int());
	
	SlcApp_Update_Config_PeriodFirstUploadSec(SlcResource[residx]->get_value_int());
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_PeriodCycleUploadSec_PutCb(void *)
{
	int residx;

	residx = SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC;
	LWM2MDATA_DEBUG("%s PUT received: %d", SlcResource_Name[residx], SlcResource[residx]->get_value_int());
	
	SlcApp_Update_Config_PeriodCycleUploadSec(SlcResource[residx]->get_value_int());
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_PeriodNodeRegisterSec_PutCb(void *)
{
	int residx;

	residx = SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC;
	LWM2MDATA_DEBUG("%s PUT received: %d", SlcResource_Name[residx], SlcResource[residx]->get_value_int());
	
	SlcApp_Update_Config_PeriodNodeRegisterSec(SlcResource[residx]->get_value_int());
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_PeriodStatusSec_PutCb(void *)
{
	int residx;

	residx = SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC;
	LWM2MDATA_DEBUG("%s PUT received: %d", SlcResource_Name[residx], SlcResource[residx]->get_value_int());
	
	SlcApp_Update_Config_PeriodStatusSec(SlcResource[residx]->get_value_int());
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_PeriodDataSec_PutCb(void *)
{
	int residx;

	residx = SLC_RESIDX_CONFIG_PERIOD_DATA_SEC;
	LWM2MDATA_DEBUG("%s PUT received: %d", SlcResource_Name[residx], SlcResource[residx]->get_value_int());
	
	SlcApp_Update_Config_PeriodDataSec(SlcResource[residx]->get_value_int());
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_DeregKeepOnline_PutCb(void *)
{
	int residx;

	residx = SLC_RESIDX_CONFIG_DEREG_KEEPONLINE;
	LWM2MDATA_DEBUG("%s PUT received: %d", SlcResource_Name[residx], SlcResource[residx]->get_value_int());
	
	SlcApp_Update_Config_DeregKeepOnline(SlcResource[residx]->get_value_int());
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_DeregKeepOnlineTimeSec_PutCb(void *)
{
	int residx;

	residx = SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC;
	LWM2MDATA_DEBUG("%s PUT received: %d", SlcResource_Name[residx], SlcResource[residx]->get_value_int());
	
	SlcApp_Update_Config_DeregKeepOnlineTimeSec(SlcResource[residx]->get_value_int());
	SlcApp_KickAppTimer();
}
// static void SlcRes_LightDim_PutCb(void *)
// {
// 	int residx;

// 	residx = SLC_RESIDX_LIGHT_DIM;
// 	LWM2MDATA_DEBUG("%s PUT received: %s", SlcResource_Name[residx], SlcResource[residx]->get_value_string().c_str());

// 	uint32_t strsize, i, totransfer;
// 	char strbuff[SLCAPP_LIGHTDIMSTRUCT_LEN*2];
// 	uint8_t buff[SLCAPP_LIGHTDIMSTRUCT_LEN];
// 	uint8_t hi ,lo;
// 	strsize = strlen(SlcResource[residx]->get_value_string().c_str());
// 	totransfer = (strsize > sizeof(buff)*2)? sizeof(buff) : strsize/2;
// 	if(strsize > sizeof(buff)*2)
// 	{
// 		printf("[SlcLwm2mDat] SlcRes_LightDim_PutCb: buff size not enough!!!\n");
// 	}
// 	memcpy(strbuff, SlcResource[residx]->get_value_string().c_str(), totransfer*2);
// 	//LWM2MDATA_DEBUG("strbuff[%d] %s", totransfer*2, strbuff);
// 	memset(buff, 0, sizeof(buff));
// 	for(i=0; i<totransfer; i++)
// 	{
// 		if( (strbuff[i*2] >= '0') && (strbuff[i*2] <= '9') )
// 			hi = strbuff[i*2] - '0';
// 		else if( (strbuff[i*2] >= 'A') && (strbuff[i*2] <= 'F') )
// 			hi = strbuff[i*2] - 'A' + 10;
// 		else if( (strbuff[i*2] >= 'a') && (strbuff[i*2] <= 'f') )
// 			hi = strbuff[i*2] - 'a' + 10;
// 		else
// 			hi = 0;

// 		if( (strbuff[i*2+1] >= '0') && (strbuff[i*2+1] <= '9') )
// 			lo = strbuff[i*2+1] - '0';
// 		else if( (strbuff[i*2+1] >= 'A') && (strbuff[i*2+1] <= 'F') )
// 			lo = strbuff[i*2+1] - 'A' + 10;
// 		else if( (strbuff[i*2+1] >= 'a') && (strbuff[i*2+1] <= 'f') )
// 			lo = strbuff[i*2+1] - 'a' + 10; 	
// 		else
// 			lo = 0;

// 		buff[i] = (hi << 4) + lo;
// 		//LWM2MDATA_DEBUG("[%d] [%x][%x] -> %x %x -> %02x", i, strbuff[i*2], strbuff[i*2+1], hi, lo, buff[i]);
// 	}
// 	//LWM2MDATA_DUMP("Dim Buff Dump", i, totransfer, buff);
// 	SlcApp_Update_LightDim(buff, totransfer);

// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_LightSchedule_PutCb(void *)
// {

// 	int residx;

// 	residx = SLC_RESIDX_LIGHT_LIGHTSCHEDULE;
// 	LWM2MDATA_DEBUG("%s PUT received: %s", SlcResource_Name[residx], SlcResource[residx]->get_value_string().c_str());
	
// 	uint32_t strsize, i, totransfer;
// 	char strbuff[SLCAPP_LIGHTSCHEDULEGROUP_LEN*2];
// 	uint8_t buff[SLCAPP_LIGHTSCHEDULEGROUP_LEN];
// 	uint8_t hi ,lo;
// 	strsize = strlen(SlcResource[residx]->get_value_string().c_str());
// 	totransfer = (strsize > sizeof(buff)*2)? sizeof(buff) : strsize/2;
// 	if(strsize > sizeof(buff)*2)
// 	{
// 		printf("[SlcLwm2mDat] SlcRes_LightSchedule_PutCb: buff size not enough!!!\n");
// 	}
// 	memcpy(strbuff, SlcResource[residx]->get_value_string().c_str(), totransfer*2);
// 	//LWM2MDATA_DEBUG("strbuff[%d] %s", totransfer*2, strbuff);
// 	memset(buff, 0, sizeof(buff));
// 	for(i=0; i<totransfer; i++)
// 	{
// 		if( (strbuff[i*2] >= '0') && (strbuff[i*2] <= '9') )
// 			hi = strbuff[i*2] - '0';
// 		else if( (strbuff[i*2] >= 'A') && (strbuff[i*2] <= 'F') )
// 			hi = strbuff[i*2] - 'A' + 10;
// 		else if( (strbuff[i*2] >= 'a') && (strbuff[i*2] <= 'f') )
// 			hi = strbuff[i*2] - 'a' + 10;
// 		else
// 			hi = 0;

// 		if( (strbuff[i*2+1] >= '0') && (strbuff[i*2+1] <= '9') )
// 			lo = strbuff[i*2+1] - '0';
// 		else if( (strbuff[i*2+1] >= 'A') && (strbuff[i*2+1] <= 'F') )
// 			lo = strbuff[i*2+1] - 'A' + 10;
// 		else if( (strbuff[i*2+1] >= 'a') && (strbuff[i*2+1] <= 'f') )
// 			lo = strbuff[i*2+1] - 'a' + 10;		
// 		else
// 			lo = 0;

// 		buff[i] = (hi << 4) + lo;
// 		//LWM2MDATA_DEBUG("[%d] [%x][%x] -> %x %x -> %02x", i, strbuff[i*2], strbuff[i*2+1], hi, lo, buff[i]);
// 	}
// 	//LWM2MDATA_DUMP("Schedule Buff Dump", i, totransfer, buff);
// 	SlcApp_Update_LightSchedule(buff, totransfer);

// 	SlcApp_KickAppTimer();
// }
static void SlcRes_CustomAtCmd_PutCb(void *)
{
	int residx;

	residx = SLC_RESIDX_ETC_CUSTOMATCMD;
	LWM2MDATA_DEBUG("%s PUT received: %s", SlcResource_Name[residx], SlcResource[residx]->get_value_string().c_str());
	
	SlcApp_Update_CustomAtCmd((char *)SlcResource[residx]->get_value_string().c_str());
	SlcApp_KickAppTimer();
}


//-----------------------------------------------------------------------------------------//
// resource post callback function
// to upload value of system to pelion
//-----------------------------------------------------------------------------------------//
static void SlcRes_NodeRegister_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_NODEREG]);
	SlcApp_Upload_NodeRegister();
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_CONFIG]);
	SlcApp_Upload_Config();
	SlcApp_KickAppTimer();
}
// static void SlcRes_ThresholdVolatgeHigh_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_TH_VOLTAGE_H]);
// 	SlcApp_Upload_ThresholdVolatgeHigh();
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdVolatgeLow_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_TH_VOLTAGE_L]);
// 	SlcApp_Upload_ThresholdVolatgeLow();
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdCurrentHigh_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_TH_CURRENT_H]);
// 	SlcApp_Upload_ThresholdCurrentHigh();
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdCurrentLow_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_TH_CURRENT_L]);
// 	SlcApp_Upload_ThresholdCurrentLow();
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdFreqHigh_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_TH_FREQ_H]);
// 	SlcApp_Upload_ThresholdFreqHigh();
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdFreqLow_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_TH_FREQ_L]);
// 	SlcApp_Upload_ThresholdFreqLow();
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdPfHigh_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_TH_PF_H]);
// 	SlcApp_Upload_ThresholdPfHigh();
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdPfLow_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_TH_PF_L]);
// 	SlcApp_Upload_ThresholdPfLow();
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdWattHigh_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_TH_WATT_H]);
// 	SlcApp_Upload_ThresholdWattHigh();
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdWattLow_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_TH_WATT_L]);
// 	SlcApp_Upload_ThresholdWattLow();
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdLuxHigh_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_TH_LUX_H]);
// 	SlcApp_Upload_ThresholdLuxHigh();
// 	SlcApp_KickAppTimer();
// }
// static void SlcRes_ThresholdLuxLow_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_TH_LUX_L]);
// 	SlcApp_Upload_ThresholdLuxLow();
// 	SlcApp_KickAppTimer();
// }
static void SlcRes_Config_WatchdogSysSec_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC]);
	SlcApp_Upload_Config_WatchdogSysSec();
	SlcApp_KickAppTimer();
}

static void SlcRes_Config_WatchdogRegSec_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC]);
	SlcApp_Upload_Config_WatchdogRegSec();
	SlcApp_KickAppTimer();
}

static void SlcRes_Config_WatchdogLightSensorSec_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC]);
	SlcApp_Upload_Config_WatchdogLightSensorSec();
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_PeriodFirstUploadSec_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC]);
	SlcApp_Upload_Config_PeriodFirstUploadSec();
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_PeriodCycleUploadSec_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC]);
	SlcApp_Upload_Config_PeriodCycleUploadSec();
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_PeriodNodeRegisterSec_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC]);
	SlcApp_Upload_Config_PeriodNodeRegisterSec();
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_PeriodStatusSec_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC]);
	SlcApp_Upload_Config_PeriodStatusSec();
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_PeriodDataSec_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_PERIOD_DATA_SEC]);
	SlcApp_Upload_Config_PeriodDataSec();
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_DeregKeepOnline_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_DEREG_KEEPONLINE]);
	SlcApp_Upload_Config_DeregKeepOnline();
	SlcApp_KickAppTimer();
}
static void SlcRes_Config_DeregKeepOnlineTimeSec_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC]);
	SlcApp_Upload_Config_DeregKeepOnlineTimeSec();
	SlcApp_KickAppTimer();
}
static void SlcRes_Status_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_INFO_STATUS]);	
	SlcApp_Upload_Status();
	SlcApp_KickAppTimer();
}
static void SlcRes_InfoNodeVersion_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_INFO_NODEVERSION]);	
	SlcApp_Upload_NodeVersion();
	SlcApp_KickAppTimer();
}
static void SlcRes_InfoMcuVersion_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_INFO_MCUVERSION]);
	SlcApp_Upload_McuVersion();
	SlcApp_KickAppTimer();
}
static void SlcRes_InfoImei_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_INFO_IMEI]);
	SlcApp_Upload_Imei();
	SlcApp_KickAppTimer();
}
static void SlcRes_InfoImsi_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_INFO_IMSI]);
	SlcApp_Upload_Imsi();
	SlcApp_KickAppTimer();
}
#ifdef _SIGNALPOWERLIGHT_RESOURCES
static void SlcRes_InfoRssi_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_INFO_RSSI]);
	SlcApp_Upload_Rssi();
	SlcApp_KickAppTimer();
}
#endif
static void SlcRes_InfoModFwVersion_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_INFO_MODFWVERSION]);
	SlcApp_Upload_ModFwVersion();
	SlcApp_KickAppTimer();
}
#ifdef _SIGNALPOWERLIGHT_RESOURCES
static void SlcRes_InfoCellId_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_INFO_CELLID]);
	SlcApp_Upload_CellId();
	SlcApp_KickAppTimer();
}
static void SlcRes_InfoRsrp_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_INFO_RSRP]);
	SlcApp_Upload_Rsrp();
	SlcApp_KickAppTimer();
}
static void SlcRes_InfoRsrq_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_INFO_RSRQ]);
	SlcApp_Upload_Rsrq();
	SlcApp_KickAppTimer();
}
static void SlcRes_InfoSnr_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_INFO_SNR]);
	SlcApp_Upload_Snr();
	SlcApp_KickAppTimer();
}
#endif
static void SlcRes_InfoCcid_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_INFO_CCID]);
	SlcApp_Upload_Ccid();
	SlcApp_KickAppTimer();
}
#ifdef _SIGNALPOWERLIGHT_RESOURCES
static void SlcRes_PowerVol_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_POWER_VOL]);
	SlcApp_Upload_PowerVol();
	SlcApp_KickAppTimer();
}
static void SlcRes_PowerFreq_PostCb(void *)
{
	int residx = SLC_RESIDX_POWER_FREQ;
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[residx]);
	SlcApp_Upload_PowerFreq();
	SlcApp_KickAppTimer();
}
static void SlcRes_PowerPf_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_POWER_PF]);
	SlcApp_Upload_PowerPF();
	SlcApp_KickAppTimer();
}
static void SlcRes_PowerCurrent_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_POWER_CURRENT]);
	SlcApp_Upload_PowerCurrent();
	SlcApp_KickAppTimer();
}
static void SlcRes_PowerWatt_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_POWER_WATT]);
	SlcApp_Upload_PowerWatt();
	SlcApp_KickAppTimer();
}
static void SlcRes_PowerTotalWatt_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_POWER_TOTALWATT]);
	SlcApp_Upload_PowerTotalWatt();
	SlcApp_KickAppTimer();
}
#endif
// static void SlcRes_LightDim_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_LIGHT_DIM]);
// 	SlcApp_Upload_LightDim();
// 	SlcApp_KickAppTimer();
// }
#ifdef _SIGNALPOWERLIGHT_RESOURCES
static void SlcRes_LightLux_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_LIGHT_LUX]);
	SlcApp_Upload_LightLux();
	SlcApp_KickAppTimer();
}
static void SlcRes_McuMode_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_LIGHT_MCUMODE]);
	SlcApp_Upload_McuMode();
	SlcApp_KickAppTimer();
}
#endif
// static void SlcRes_LightSchedule_PostCb(void *)
// {
// 	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_LIGHT_LIGHTSCHEDULE]);
// 	SlcApp_Upload_LightSchedule();
// 	SlcApp_KickAppTimer();
// }
static void SlcRes_SignalPowerLight_PostCb(void *)
{
	LWM2MDATA_DEBUG("%s POST received", SlcResource_Name[SLC_RESIDX_ETC_SIGNALPOWERLIGHT]);
	SlcApp_Upload_SignalPowerLight();
	SlcApp_KickAppTimer();
}


//-----------------------------------------------------------------------------------------//
// resource status callback function
// to check if resourced subscribed
//-----------------------------------------------------------------------------------------//
static void SlcRes_status_callback(const M2MBase& object,
                            const M2MBase::MessageDeliveryStatus status,
                            const M2MBase::MessageType /*type*/)
{
    switch(status) {
        case M2MBase::MESSAGE_STATUS_BUILD_ERROR:
            printf("SLC Message status callback: %d (%s) error when building CoAP message\n", status, object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_RESEND_QUEUE_FULL:
            printf("SLC Message status callback: %d (%s) CoAP resend queue full\n", status, object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_SENT:
            printf("SLC Message status callback: %d (%s) Message sent to server\n", status, object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_DELIVERED:
            printf("SLC Message status callback: %d (%s) Message delivered\n", status, object.uri_path());
			SlcApp_KickAppTimer();
            break;
        case M2MBase::MESSAGE_STATUS_SEND_FAILED:
            printf("SLC Message status callback: %d (%s) Message sending failed\n", status, object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_SUBSCRIBED:
            printf("SLC Message status callback: %d (%s) subscribed\n", status, object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_UNSUBSCRIBED:
            printf("SLC Message status callback: %d (%s) subscription removed\n", status, object.uri_path());
            break;
        case M2MBase::MESSAGE_STATUS_REJECTED:
            printf("SLC Message status callback: %d (%s) server has rejected the message\n", status, object.uri_path());
            break;
        default:
			printf("SLC Message status callback: %d\n", status);
            break;
    }
}

static void SlcRes_NodeRegister_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_CONFIG_NODEREG;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_Config_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_CONFIG_CONFIG;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
// static void SlcRes_ThresholdVolatgeHigh_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_CONFIG_TH_VOLTAGE_H;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
// static void SlcRes_ThresholdVolatgeLow_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_CONFIG_TH_VOLTAGE_L;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
// static void SlcRes_ThresholdCurrentHigh_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_CONFIG_TH_CURRENT_H;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
// static void SlcRes_ThresholdCurrentLow_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_CONFIG_TH_CURRENT_L;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
// static void SlcRes_ThresholdFreqHigh_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_CONFIG_TH_FREQ_H;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
// static void SlcRes_ThresholdFreqLow_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_CONFIG_TH_FREQ_L;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
// static void SlcRes_ThresholdPfHigh_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_CONFIG_TH_PF_H;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
// static void SlcRes_ThresholdPfLow_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_CONFIG_TH_PF_L;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
// static void SlcRes_ThresholdWattHigh_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_CONFIG_TH_WATT_H;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
// static void SlcRes_ThresholdWattLow_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_CONFIG_TH_WATT_L;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
// static void SlcRes_ThresholdLuxHigh_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_CONFIG_TH_LUX_H;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
// static void SlcRes_ThresholdLuxLow_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_CONFIG_TH_LUX_L;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
static void SlcRes_ConfigWatchdogSysSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_CONFIG_WATCHDOG_SYS_SEC;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_ConfigWatchdogRegSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_CONFIG_WATCHDOG_REG_SEC;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_ConfigWatchdogLightSensorSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_CONFIG_WATCHDOG_LIGHTSENSOR_SEC;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_ConfigPeriodFirstUploadSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_CONFIG_PERIOD_FIRSTUPLOAD_SEC;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_ConfigPeriodCycleUploadSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_CONFIG_PERIOD_CYCLEUPLOAD_SEC;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_ConfigPeriodNodeRegisterSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_CONFIG_PERIOD_NODEREGISTER_SEC;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_ConfigPeriodStatusSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_CONFIG_PERIOD_STATUS_SEC;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_ConfigPeriodDataSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_CONFIG_PERIOD_DATA_SEC;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_ConfigDeregKeepOnline_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_CONFIG_DEREG_KEEPONLINE;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_ConfigDeregKeepOnlineTimeSec_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_CONFIG_DEREG_KEEPONLINETIME_SEC;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_InfoStatus_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_INFO_STATUS;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_InfoNodeVersion_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_INFO_NODEVERSION;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_InfoMcuVersion_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_INFO_MCUVERSION;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_InfoImei_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_INFO_IMEI;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_InfoImsi_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_INFO_IMSI;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
#ifdef _SIGNALPOWERLIGHT_RESOURCES
static void SlcRes_InfoRssi_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_INFO_RSSI;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
#endif
static void SlcRes_InfoModFwVersion_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_INFO_MODFWVERSION;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
#ifdef _SIGNALPOWERLIGHT_RESOURCES
static void SlcRes_InfoCellId_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_INFO_CELLID;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_InfoRsrp_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_INFO_RSRP;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_InfoRsrq_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_INFO_RSRQ;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_InfoSnr_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_INFO_SNR;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
#endif
static void SlcRes_InfoCcid_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_INFO_CCID;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
#ifdef _SIGNALPOWERLIGHT_RESOURCES
static void SlcRes_PowerVol_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_POWER_VOL;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_PowerFreq_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_POWER_FREQ;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_PowerPf_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_POWER_PF;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_PowerCurrent_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_POWER_CURRENT;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}

static void SlcRes_PowerWatt_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_POWER_WATT;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_PowerTotalWatt_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_POWER_TOTALWATT;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
#endif
// static void SlcRes_LightDim_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_LIGHT_DIM;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
#ifdef _SIGNALPOWERLIGHT_RESOURCES
// static void SlcRes_LightLux_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_LIGHT_LUX;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
static void SlcRes_McuMode_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_LIGHT_MCUMODE;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
#endif
// static void SlcRes_LightSchedule_StatusCb(const M2MBase& object,
// 									const M2MBase::MessageDeliveryStatus status,
// 									const M2MBase::MessageType type)
// {
// 	int i = SLC_RESIDX_LIGHT_LIGHTSCHEDULE;
// 	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
// 		SlcResource_Subscribed[i] = true;
// 	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
// 	SlcRes_status_callback(object, status, type);
// }
static void SlcRes_SignalPowerLight_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_ETC_SIGNALPOWERLIGHT;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}
static void SlcRes_CustomAtCmd_StatusCb(const M2MBase& object,
									const M2MBase::MessageDeliveryStatus status,
									const M2MBase::MessageType type)
{
	int i = SLC_RESIDX_ETC_CUSTOMATCMD;
	if(status == M2MBase::MESSAGE_STATUS_SUBSCRIBED)
		SlcResource_Subscribed[i] = true;
	printf("StatusCb: %d %s\n", i, SlcResource_Name[i]);
	SlcRes_status_callback(object, status, type);
}


void SlcLwm2mDate_ResetResourceSubscribed(void)
{
	int i;
	memset(SlcResource_Subscribed, false, sizeof(SlcResource_Subscribed));

	for(i=0; i<SLC_RES_MAX; i++)
	{
		if(SlcResource_AutoObservable[i])
		{
			SlcResource_Subscribed[i] = true;
		}
	}
}

bool SlcLwm2mDate_IsAllResSubscribed(void)
{
	int i;
	for(i=0; i<SLC_RES_MAX; i++)
		if(SlcResource_Subscribed[i] == false)
			return false;

	return true;
}

bool *SlcLwm2mDate_ResSubscribedStatus(int *res_cnt)
{
	*res_cnt = SLC_RES_MAX;
	return SlcResource_Subscribed;
}

#endif // #ifdef _PROJECT_SLC

