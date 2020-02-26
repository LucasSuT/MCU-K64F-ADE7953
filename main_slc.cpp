// ----------------------------------------------------------------------------
// Copyright 2016-2019 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------
#include "common_def.h"

#if defined(_PROJECT_SLC) || defined(_PROJECT_SPM)

#include "common_debug.h"
#ifdef _DEBUG_MAIN
#define TAG				"MAIN"
#define MAIN_DEBUG		DEBUGMSG
#define MAIN_DUMP		DEBUGMSG_DUMP
#else
#define MAIN_DEBUG
#define MAIN_DUMP
#endif //_DEBUG_SCLCTRL
#define MAIN_ERR		DEBUGMSG_ERR

#include "simplem2mclient.h"
#ifdef TARGET_LIKE_MBED
#include "mbed.h"
#endif
#include "pal.h"
#include "application_init.h"
//#include "mcc_common_button_and_led.h"
//#include "blinky.h"
#ifndef MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT
#include "certificate_enrollment_user_cb.h"
#endif

#include "nsapi_types.h"						// nsapi error code
#include "ntp-client/NTPClient.h"				// sync time
#include "CellularContext.h"					// CellularContext
#include "CellularNetwork.h"					// CellularNetwork
#include "CellularExternalControl.h"
#include "SlcApplication.h"
#include "SlcLwm2mData.h"
#include "SlcControl.h"
#include "led.h"


//---------------------------------------------------------------------//
//	define constant
//---------------------------------------------------------------------//
// date and time
#define NTP_DATETIME_BASE_YEAR		1900
#define NTP_DATETIME_BASE_MONTH		1

#define DATETIME_THRESHOLD_YEAR		2018	// RTC year should be larger than this value, and that can be verified as real time year
#define IF_REAL_RTC(year)			(year > DATETIME_THRESHOLD_YEAR)? true:false


//---------------------------------------------------------------------//
//	function declaration
//---------------------------------------------------------------------//
static void main_application(void);
static int SyncRealTime(bool *terminate);

static void Watchdog_InitStart(void);
static void Watchdog_TimerFxn(void const *n);

extern int factory_flow_task(bool ForseErase);				// merge fce


static void Main_TimerThreadWatchdogFxn(void const *n);
static void Main_KickThreadTimer(void);

//---------------------------------------------------------------------//
//	variables declaration
//---------------------------------------------------------------------//
typedef struct {
	bool TimerThreadWatchdog;	// flag for launch timer

	// action
	bool ToNetDisconnect;
	bool ToNetConnect;
	// config
	_DisconnectType DisconnectType;
	// status
	_NetState NetState;
	bool PelionRegistered;
	bool Paused;
	// record
	char DeviceIDStr[40];
} _MainStruct;

static _MainStruct Main;

// Pointer to mbedClient, used for calling close function.
static SimpleM2MClient *Client;

static M2MResource* factoryreset_res;

//---------------------------------------------------------------------//
// watchdog
//---------------------------------------------------------------------//
#define DEFAULT_WATCHDOG_PRESCALER				IWDG_PRESCALER_256	// max value for stm32f429zi prescaler is 256 (8bits) --> IWDG_PRESCALER_256
																	// 1 count for: 1/(32kHz/256) = 1/(32*1024/256) = 1/(32*4) = 1/128 sec
#define DEFAULT_WATCHDOG_COUNT					0x500				// max value for stm32f429zi watchdog counter is 0xFFF (12bits)
																	// 0xFFF: 4095/128 = 31.992 sec
																	// 0x500: 1280/128 = 10 sec
#define DEFAULT_WATCHDOG_KICKPERIOD_SEC			8					// this value must be less than watchdog timeout time: 1/(32kHz/prescaler)*count

IWDG_HandleTypeDef IwdgHandle;

// timer for system watchdog
RtosTimer WatchdogTimer(Watchdog_TimerFxn, osTimerPeriodic, (void *)0);

RtosTimer Main_TimerThreadWatchdog(Main_TimerThreadWatchdogFxn, osTimerPeriodic, (void *)0);

//-----------------------------------------------------------------------------------//
// macro declaration
//-----------------------------------------------------------------------------------//
// timer - thread watchdog
#define MAIN_RELAUNCHTIMER_THREADWATCHDOG()	do { \
													if(Main.TimerThreadWatchdog) Main_TimerThreadWatchdog.stop(); \
													Main_TimerThreadWatchdog.start(MAINTHREAD_WATCHDOG_TIMEOUTTIME_SEC*1000); \
													Main.TimerThreadWatchdog = true; \
												} while(0)


//---------------------------------------------------------------------//
//	callback function
//---------------------------------------------------------------------//
//--------------------------------------//
// thread watchdog functiions
//--------------------------------------//
static void Main_TimerThreadWatchdogFxn(void const *n)
{
	_DateTimeStruct now;
	RTC_GetRealTime(&now);
													
	Main.TimerThreadWatchdog = false;
													
	MAIN_DEBUG("!!!!!!!! Main_TimerThreadWatchdogFxn !!!!!!!!!!!!!!!!!!!");
	MAIN_DEBUG("@ %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
	OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
}
												
static void Main_KickThreadTimer(void)
{
	//SLCCTRL_DEBUG("reset thread-watchdog timer");
	MAIN_RELAUNCHTIMER_THREADWATCHDOG();
}

//--------------------------------------//
// mbed-cloud-client registration callback
//--------------------------------------//
/**
 * Registration callback handler
 * @param endpoint Information about the registered endpoint such as the name (so you can find it back in portal)
 */
void registered_callback(void *dev_name)
{
    time_t timestamp = (time_t)time(NULL);
    struct tm *t = localtime(&timestamp);
    char buffer[32];
    memset(buffer, 0, 32);
    strftime(buffer, 32, "%FT%T\n", t);
    for(int i=0; i<32; i++)
    {
        if( (buffer[i]=='\r') || (buffer[i]=='\n') )
            buffer[i] = 0;
    }

	strcpy(Main.DeviceIDStr, (char *)dev_name);
	
	printf("\n\n\n***********************************************\n");
	printf("***********************************************\n");
	printf("%s registered at %s\n", Main.DeviceIDStr, buffer);
	printf("***********************************************\n");
	printf("***********************************************\n\n");

	Main.PelionRegistered = true;
}

//--------------------------------------//
// resource callback
//--------------------------------------//
// This function is called when a POST request is received for resource 5000/0/1.
/*
void unregister(void *)
{
    printf("Unregister resource executed\n");
    Client->close();
}
*/

// This function is called when a POST request is received for resource 5000/0/2.
void factory_reset(void *)
{
	int64_t factory_reset_type;
	
    factory_reset_type = factoryreset_res->get_value_int();
    MAIN_DEBUG("factory_reset received, value: %d", factory_reset_type);
	if(factory_reset_type == OP_REBOOT)
	{
		MAIN_DEBUG("do reboot");
		OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
	}
	else if(factory_reset_type == OP_CONFIG_FACTORY_RESET)
	{
		MAIN_DEBUG("do config factory reset");
		SlcApp_ResetConfig();
		//OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
		factoryreset_res->set_value(0);
	}
	/*
	else if(factory_reset_type == OP_KCM_FACTORY_RESET)
	{
		MAIN_DEBUG("do kcm factory reset");
		MAIN_DEBUG("Factory reset resource executed");
	    Client->close();
	    kcm_status_e kcm_status = kcm_factory_reset();
	    if (kcm_status != KCM_STATUS_SUCCESS) {
	        MAIN_DEBUG("Failed to do factory reset - %d", kcm_status);
	    } else {
	        MAIN_DEBUG("Factory reset completed. Now restart the device");
	    }
		OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
	}
	*/
}

static void nw_callback(nsapi_event_t ev, intptr_t intptr)
{
}

//---------------------------------------------------------------------//
//	functions
//---------------------------------------------------------------------//
int main(void)
{
	//CellularExt_HWReset_Reset();

	#if defined(_DEVICE_QUECTEL_BG96) || defined(_DEVICE_UBLOX_SARA_R410M)
	CellularExt_HWReset_Set(1); // for bg96 and ublox sara-n410, default high
	#endif	

	#if defined(_DEVICE_QUECTEL_BG96)
		#if defined(_CELLULAR_DEVICE_NBIOT)
			printf("Firmware Version: %d.%d.%d BG96-NBIOT\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB);
		#elif defined(_CELLULAR_DEVICE_CATM1)
			printf("Firmware Version: %d.%d.%d BG96-CATM1\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB);
		#elif defined(_CELLULAR_DEVICE_AUTO)
			printf("Firmware Version: %d.%d.%d BG96-AUTO\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB);
		#else
			printf("Firmware Version: %d.%d.%d BG96-NonDefine\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB);
		#endif
	#elif defined(_DEVICE_QUECTEL_BC95)
		#if defined(_CELLULAR_DEVICE_BAND28)
			printf("Firmware Version: %d.%d.%d BC95-G Band 28 Only\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB);
		#elif defined(_CELLULAR_DEVICE_BAND3_8_28)
			printf("Firmware Version: %d.%d.%d BC95-G Band 3+8+28 Only\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB);
		#elif defined(_CELLULAR_DEVICE_BANDFULL)
			printf("Firmware Version: %d.%d.%d BC95-G Full Band\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB);
		#else
			printf("Firmware Version: %d.%d.%d BC95-G\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB);
		#endif
	#elif defined(_DEVICE_UBLOX_SARA_R410M)
		#if defined(_CELLULAR_DEVICE_BAND28)
			printf("Firmware Version: %d.%d.%d UBLOX SARA-R410M Band 28 Only\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB);
		#elif defined(_CELLULAR_DEVICE_BAND3_8_28)
			printf("Firmware Version: %d.%d.%d UBLOX SARA-R410M Band 3+8+28 Only\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB);
		#elif defined(_CELLULAR_DEVICE_BANDFULL)
			printf("Firmware Version: %d.%d.%d UBLOX SARA-R410M Full Band\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB);
		#else
			printf("Firmware Version: %d.%d.%d UBLOX SARA-R410M\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB);
		#endif
	#else
		printf("Firmware Version: %d.%d.%d\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE, AAEON_FW_VERSION_SUB);
	#endif

	#ifdef _MBEDCLOUDCLIENT_PAUSERESUME
		printf("Pause mbed-cloud-client for disconnect\n");
	#else
		printf("Close mbed-cloud-client for disconnect\n");
	#endif

	if(DEFAULT_DISCONNECT_TYPE == DISCONNECT_JUSTNETWORK)
		printf("just close network\n");
	else if(DEFAULT_DISCONNECT_TYPE == DISCONNECT_RAI)
		printf("close network and send RAI flag\n");
	else if(DEFAULT_DISCONNECT_TYPE == DISCONNECT_DETACH)
		printf("close network and detach\n");

	#if defined(MBED_CONF_APP_DEVELOPER_MODE) && (MBED_CONF_APP_DEVELOPER_MODE == 1)
		printf("developer mode !!!\n");
	#else
		printf("production mode !!!\n");
	#endif

	#ifdef _OS_REBOOT_WITH_MODEM_SW_RESET
		printf("reboot os with modem SW reset\n");
	#elif defined(_OS_REBOOT_WITH_MODEM_HW_RESET)
		printf("reboot os with modem HW reset\n");
	#else
		printf("reboot os without modem reset\n");
	#endif
	
    return mcc_platform_run_program(main_application);
}

void main_application(void)
{
	int ret;
	
	printf("\n================================================================\n");
	printf("FW Version: %d.%d\n", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE);
	printf("Date & Time: %s - %s\n", __DATE__, __TIME__);
	printf("================================================================\n\n");
	#if defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP)
	printf("MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP\n");
	#elif defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP)
	printf("MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP\n");
	#elif defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE)
	printf("MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE\n");
	#endif
	printf("SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE: %d\n", SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE);
	
	printf("\n=============================\n");
	RTC_PrintRealTime();
	printf("=============================\n\n");
    
	#if defined(__linux__) && (MBED_CONF_MBED_TRACE_ENABLE == 0)
    // make sure the line buffering is on as non-trace builds do
    // not produce enough output to fill the buffer
    setlinebuf(stdout);
	#endif


	//---------------------------------------------------------------------//
	// Initialize led
	//---------------------------------------------------------------------//
	LED_RED_ON();
	LED_GREEN_OFF();

	//---------------------------------------------------------------------//
	// Initialize trace-library first
	//---------------------------------------------------------------------//
	if (application_init_mbed_trace() != 0) {
		printf("Failed initializing mbed trace\n" );
		LED_RED_BLINK_FAST();
		//while(1);
		wait(10);
		OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
	}

	// Print platform information
	mcc_platform_sw_build_info();

    //---------------------------------------------------------------------//
    //  factory flow
    //---------------------------------------------------------------------//
	#ifdef _PROJECT_SPM_FCU_INCODE
	printf("\n================================================================\n");
	printf("factory flow task, force to erase\n");
	factory_flow_task(false);	// sophie, merge fce
	wait(2);
	printf("================================================================\n\n");
	#endif
	
    //---------------------------------------------------------------------//
    //  launch communication with SLC control
    //---------------------------------------------------------------------//
    printf("\n=============================\n");
	SlcCtrl_Init();
	wait(2);
	printf("=============================\n\n");

	//---------------------------------------------------------------------//
	// Initialize storage
	//---------------------------------------------------------------------//
	if (mcc_platform_storage_init() != 0) {
		printf("Failed to initialize storage\n" );
		LED_RED_BLINK_FAST();
		//while(1);
		wait(10);
		OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
	}

	//---------------------------------------------------------------------//
	// Initialize platform-specific components
	//---------------------------------------------------------------------//
	if(mcc_platform_init() != 0) {
		printf("ERROR - platform_init() failed!\n");
		LED_RED_BLINK_FAST();
		//while(1);
		wait(10);
		OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
	}	

    //---------------------------------------------------------------------//
    //  initialize variables
    //---------------------------------------------------------------------//
	Main.ToNetDisconnect	= false;
	Main.ToNetConnect		= false;
	Main.DisconnectType		= DEFAULT_DISCONNECT_TYPE;
	Main.NetState			= NETSTATE_DISCONNECTED;
	Main.PelionRegistered	= false;
	Main.Paused				= false;
	memset(Main.DeviceIDStr, 0, sizeof(Main.DeviceIDStr));

    //---------------------------------------------------------------------//
    // launch system watchdog
    //---------------------------------------------------------------------//
    printf("\n=============================\n");
	printf("launch watchdog\n");
	printf("=============================\n\n");
	Watchdog_InitStart();

    // Print some statistics of the object sizes and their heap memory consumption.
    // NOTE: This *must* be done before creating MbedCloudClient, as the statistic calculation
    // creates and deletes M2MSecurity and M2MDevice singleton objects, which are also used by
    // the MbedCloudClient.
	#ifdef MBED_HEAP_STATS_ENABLED
    print_m2mobject_stats();
	#endif

	//---------------------------------------------------------------------//
	//  initialize mbed-cloud-client
	//---------------------------------------------------------------------//
	printf("\n=============================\n");
	printf("initialize mbed-cloud-client\n");
	printf("=============================\n\n");
	// SimpleClient is used for registering and unregistering resources to a server.
	SimpleM2MClient mbedClient;

    // application_init() runs the following initializations:
    //  1. platform initialization
    //  2. print memory statistics if MBED_HEAP_STATS_ENABLED is defined
    //  3. FCC initialization.
	printf("\n=============================\n");
	printf("initialize application\n");
	printf("=============================\n\n");
    if (!application_init()) {
        printf("Initialization failed, exiting application!\n");
		LED_RED_BLINK_FAST();
		//while(1);
		wait(10);
		OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
    }

    // Save pointer to mbedClient so that other functions can access it.
    Client = &mbedClient;

	Main_PrintHeapStack("main basic initialized");

	//---------------------------------------------------------------------//
    //  create resources
    //---------------------------------------------------------------------//
    //---------------------------------------------------------------------//
    //  create resources - example : remove in future!!!
    //---------------------------------------------------------------------//
	#ifndef MCC_MEMORY
	printf("\n=============================\n");
	printf("create resources\n");
	printf("=============================\n\n");
    // Create resource for unregistering the device. Path of this resource will be: 5000/0/1.
    //mbedClient.add_cloud_resource(5000, 0, 1, "unregister", M2MResourceInstance::STRING,
    //             M2MBase::POST_ALLOWED, NULL, false, NULL, (void*)unregister, NULL);

    // Create resource for running factory reset for the device. Path of this resource will be: 5000/0/2.
	factoryreset_res = mbedClient.add_cloud_resource(5000, 0, 2, "factory_reset", M2MResourceInstance::INTEGER,
				M2MBase::GET_PUT_ALLOWED, NULL, false, (void*)factory_reset, NULL, NULL);
	#endif

    //---------------------------------------------------------------------//
    //  create resources - Izanagi
    //---------------------------------------------------------------------//
	SlcLwm2mDate_CreateResource(&mbedClient);

    //---------------------------------------------------------------------//
    // Initialize network
    //---------------------------------------------------------------------//
	printf("\n=============================\n");
	printf("Initialize cellular module\n");
	printf("=============================\n\n");
	ret = CellularExt_PreInitModule();
	if(ret == 1)
	{
		//CellularExt_HWReset_Reset();
		OS_Reboot(DEFAULT_INIT_FAIL_WITH_MODEM_RESET_CONFIG);
	}
	else if(ret == -1)
	{
		printf("Failed to initialize cellular module\n");
		LED_RED_BLINK_FAST();
		//while(1);
		//CellularExt_HWReset_Reset();
		OS_Reboot(DEFAULT_INIT_FAIL_WITH_MODEM_RESET_CONFIG);
	}
	//bool psm;
	//CellularExt_GetPsm(&psm);
	//printf("psm: %d\n", psm);
	//bool edrx;
	//CellularExt_GetEdrx(&edrx);
	//printf("edrx: %d\n", edrx);
	//CellularExt_GetEdrxrdp();
	wait(5);

    //---------------------------------------------------------------------//
    //  Config mbed-cloud-client callback
    //---------------------------------------------------------------------//
	// Callback that fires when registering is complete
	mbedClient.on_registered(&registered_callback);

	#if 1
	//---------------------------------------------------------------------//
	// launch application thread
	//---------------------------------------------------------------------//
	printf("\n=============================\n");
	printf("initialize slc-application\n");
	printf("=============================\n\n");
	SlcApp_Init();

    //---------------------------------------------------------------------//
    //  Connect and register mbed-cloud-client
    //---------------------------------------------------------------------//
    /*
    printf("================================================================\n");
	printf("Connect and register mbed-cloud-client\n");
	printf("================================================================\n");
    mbedClient.register_and_connect();
	*/

	#ifndef MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT
    // Add certificate renewal callback
    mbedClient.get_cloud_client().on_certificate_renewal(certificate_renewal_cb);
	#endif // MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT

	Main_PrintHeapStack("start while loop");

    //---------------------------------------------------------------------//
    //  while loop
    //---------------------------------------------------------------------//
	printf("\n=============================\n");
	printf("main loop\n");
	printf("=============================\n\n");

	//CellularNetwork *nw = ((CellularContext *)mcc_platform_get_network_interface())->get_device()->open_network();
	CellularDevice *device = CellularDevice::get_default_instance();
	CellularNetwork *nw = device->open_network();

	int err_reboot = 0;
    while(1)
    {
    	Main_KickThreadTimer();
		
		if(Main.ToNetConnect)
		{
			printf("\n=============================\n");
    		printf("to connect network\n");
			if(Main.NetState != NETSTATE_CONNECTED)
			{
				Main.NetState = NETSTATE_CONNECTING;
				
				// todo: re-attach anyway to prevent module has detached and then system reboot ?
				if(Main.DisconnectType == DISCONNECT_DETACH)
				{
					printf("\n=============================\n");
		    		printf("to attach\n");
					printf("=============================\n\n");
					CellularExt_Attach();
					//nw->attach(&nw_callback); // R410M cannot attach again by this api if detach and reboot
				}
				int ret;
				int err = 0;
				while(1)
				{
					printf("\n=============================\n");
					printf("mcc_platform_init_connection\n");
					printf("=============================\n\n");
					ret = mcc_platform_init_connection();
					if(ret == 0)
					{
						printf("network established\n");
						err_reboot = 0;							
						break;
					}
					else
					{
						printf("network init fail %d, wait and retry\n", ret);
						if(ret == NETWORK_ERROR_TIMEOUT)
						{
							printf("NETWORK_ERROR_TIMEOUT\n");
							err_reboot++;							
						}
						else if(ret == NETWORK_ERROR_NO_MEMORY)
						{
							printf("NETWORK_ERROR_NO_MEMORY\n");
							err_reboot++;
						}
						else if(ret == NETWORK_ERROR_NO_NETWORK_INTERFACE)
						{
							printf("no network interface\n");
							Main.NetState = NETSTATE_ERROR_REBOOT;
							break;
						}
						else if(Main.ToNetDisconnect)
						{
							printf("Main.ToNetDisconnect triggered\n");
							break;
						}
						else
							err++;

						wait(3);
						if(err_reboot > 3)
						{
							err_reboot = 0;
							Main.NetState = NETSTATE_ERROR_REBOOT;
							break;
						}
						else if(err > 3)
						{
							err = 0;
							Main.NetState = NETSTATE_ERROR;
							break;
						}
					}
				};
				/*
				while(mcc_platform_get_network_status() != NETWORK_STATUS_GLOBAL_UP)
				{
					printf("waiting network connecting...\n");
					wait(2);
				};
				*/				
			}
			if(!Main.ToNetDisconnect)
			{
				// the status always be DISCONNECTED for 2nd connnetion and don't know why
				// so ignore this status check
				//if(mcc_platform_get_network_status() == NETWORK_STATUS_GLOBAL_UP)
				if( (Main.NetState != NETSTATE_ERROR) && (Main.NetState != NETSTATE_ERROR_REBOOT) )
				{
					printf("\n=============================\n");
					int rssi, ber;
					CellularExt_GetSignalQuality(&rssi, &ber);
					printf("RSSI: %ddBm\n", rssi);
					int rsrp, rsrq, snr;
					CellularExt_GetSignalStrength(&rsrp, &rsrq, &snr);
					printf("rsrp: %d, rsrq: %d, snr: %d\n\n", rsrp, rsrq, snr);

					uint32_t cellid;
					CellularExt_GetCellID(&cellid);
					printf("Cell ID: %d\n", cellid);

					//bool psm;
					//CellularExt_GetPsm(&psm);
					//printf("psm: %d\n", psm);

					//bool edrx;
					//CellularExt_GetEdrx(&edrx);
					//printf("edrx: %d\n", edrx);
					//CellularExt_GetEdrxrdp();
					printf("=============================\n\n");
					
					int ret;
					_DateTimeStruct now;
					RTC_GetRealTime(&now);
					if(IF_REAL_RTC(now.Y))
					{
						printf("\n=============================\n");
						printf("time already be sync\n");
						ret = 0;
					}
					else
					{
						printf("\n=============================\n");
						printf("to sync real time\n");
						ret = SyncRealTime(&Main.ToNetDisconnect);
						if(ret < 0)
							printf("to sync real time fail (%d), ignore\n", ret);
						ret = 0; //test
					}
					
					if(ret < 0)
					{
						printf("SyncRealTime error %d\n", ret);
						if(ret == NETWORK_ERROR_NO_SOCKET)
						{
							printf("SyncRealTime NETWORK_ERROR_NO_SOCKET, -->NETSTATE_ERROR\n");
							Main.NetState = NETSTATE_ERROR;
						}
						else if(ret == NETWORK_ERROR_DEVICE_ERROR)
						{
							printf("SyncRealTime NETWORK_ERROR_DEVICE_ERROR, -->NETSTATE_ERROR_REBOOT\n");
							Main.NetState = NETSTATE_ERROR_REBOOT;
						}
						else if(ret == NETWORK_ERROR_DNS_FAILURE)		// connect ntpclient fail
						{
							printf("SyncRealTime NETWORK_ERROR_DNS_FAILURE, -->NETSTATE_ERROR\n");
							Main.NetState = NETSTATE_ERROR;
						}
						else if(ret == NETWORK_ERROR_WOULD_BLOCK)
						{
							printf("SyncRealTime NETWORK_ERROR_WOULD_BLOCK, -->NETSTATE_ERROR_REBOOT\n");
							Main.NetState = NETSTATE_ERROR_REBOOT;
						}						
					}
					else if(!Main.ToNetDisconnect)
					{
						printf("real time sync\n");
						printf("=============================\n\n");
						
						Main.NetState = NETSTATE_CONNECTED;
						wait(1);
						
						printf("\n=============================\n");
						if(!Main.PelionRegistered || !Main.Paused)
						{
							printf("to register pelion\n");
							mbedClient.register_and_connect();
						}
						else
						{
							printf("to resume pelion\n");
							mbedClient.resume(mcc_platform_get_network_interface());
							Main.Paused = false;
						}
						printf("=============================\n\n");
					}
				}
			}
			Main.ToNetConnect = false;
		}

    	if(Main.ToNetDisconnect)
    	{
    		Main_PrintHeapStack("main to disconnect");
		
			#ifdef _MBEDCLOUDCLIENT_PAUSERESUME
				printf("\n=============================\n");
	    		printf("to pause/deregister pelion\n");
				printf("=============================\n\n");
				if(Client->is_client_registered())
				{
					printf("client registered, to pause\n");
		    		mbedClient.pause();
					printf("to pause pelion done\n");
					wait(3);
					Main.Paused = true;
				}
				else
				{
					printf("client un-registered, to close\n");
					mbedClient.close();
					printf("to close pelion done\n");
					int waitcnt = 0;
					while(Client->is_client_registered())
					{
						printf("waiting pelion deregistered...%d\n", waitcnt+1);
						wait(2);
						waitcnt++;
						if(waitcnt > 10)
						{
							// not reboot, ublox can not disconnect and don't know why
							//printf("waiting pelion deregistered too long, break and reboot !!!!!!!!!!\n");
							//Main.NetState = NETSTATE_ERROR_REBOOT;
							break;
						}
					}
					printf("to close pelion done\n");
					Main.PelionRegistered = false;
				}				
			#else
				printf("\n=============================\n");
	    		printf("to deregister pelion\n");
				printf("=============================\n\n");
				mbedClient.close();
				printf("to close pelion done\n");
				int waitcnt = 0;
				while(Client->is_client_registered())
				{
					printf("waiting pelion deregistered...%d\n", waitcnt+1);
					wait(2);
					waitcnt++;
					if(waitcnt > 10)
					{
						// not reboot, ublox can not disconnect and don't know why
						//printf("waiting pelion deregistered too long, break and reboot !!!!!!!!!!\n");
						//Main.NetState = NETSTATE_ERROR_REBOOT;
						break;
					}
				};
				printf("to close pelion done\n");
				Main.PelionRegistered = false;
			#endif

			printf("\n=============================\n");
    		printf("to disconnect network\n");
			printf("=============================\n\n");
			do
			{
				mcc_platform_close_connection();
				printf("waiting network disconnected...\n");
				wait(2);
				break; // sophie break for temp, ublox cannot close successfully
			} while(mcc_platform_get_network_status() != NETWORK_STATUS_DISCONNECTED);

			if(Main.NetState == NETSTATE_ERROR_REBOOT)
			{
				printf("net state error, try to reset module and reboot system\n");
				CellularExt_HWReset_Reset();
				// maybe to reset cellular state machine?
				OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
				Main.NetState = NETSTATE_DISCONNECTED;
			}
			else
			{
				if(Main.DisconnectType == DISCONNECT_RAI)
				{
					printf("\n=============================\n");
		    		printf("to disconnect RAI\n");
					printf("=============================\n\n");
					// call specific api to send RAI flag
					CellularExt_DisconnectRAI();
					Main.NetState = NETSTATE_RAIDISCONNECTED;
				}
				else if(Main.DisconnectType == DISCONNECT_DETACH)
				{
					printf("\n=============================\n");
		    		printf("to detach\n");
					printf("=============================\n\n");
					CellularExt_Detach();	// call specific api to detach
					//nw->detach();
					Main.NetState = NETSTATE_DETACHDISCONNECTED;
				}
				else
					Main.NetState = NETSTATE_DISCONNECTED;
			}

			Main.ToNetDisconnect = false;			
    	}
    }	

	#else

	printf("\n================================================================\n");
	printf("mcc_platform_init_connection\n");
	printf("\n================================================================\n");
	mcc_platform_init_connection();

	printf("\n================================================================\n");
	printf("wait 60 secs\n");
	printf("\n================================================================\n");
	int netstate;
	int j;
	for(j=0; j<30; j++)
	{
		netstate = mcc_platform_get_network_status();
		printf("net state: %d\n", netstate);
		if(netstate == NETWORK_STATUS_GLOBAL_UP)
		{
			printf("NETWORK_STATUS_GLOBAL_UP\n");
			break;
		}
		wait(2);
	}
	
    //printf("================================================================\n");
	//printf("Connect and register mbed-cloud-client\n");
	//printf("================================================================\n");
    //mbedClient.register_and_connect();

	//#ifndef MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT
    //// Add certificate renewal callback
    //mbedClient.get_cloud_client().on_certificate_renewal(certificate_renewal_cb);
	//#endif // MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT

	//printf("\n================================================================\n");
	//printf("pause\n");
	//printf("\n================================================================\n");
	//mbedClient.pause();

	//printf("\n================================================================\n");
	//printf("detach\n");
	//printf("\n================================================================\n");
	//nw->detach();

	printf("\n================================================================\n");
	printf("sync time\n");
	printf("\n================================================================\n");
	bool fterminate = false;
	SyncRealTime(&fterminate);

	printf("\n================================================================\n");
	printf("mcc_platform_close_connection\n");
	printf("\n================================================================\n");
	mcc_platform_close_connection();

	printf("\n================================================================\n");
	printf("wait 120 secs\n");
	printf("\n================================================================\n");
	for(j=0; j<30; j++)
	{
		netstate = mcc_platform_get_network_status();
		printf("net state: %d\n", netstate);
		if(netstate == NETWORK_STATUS_DISCONNECTED)
		{
			printf("NETWORK_STATUS_DISCONNECTED\n");
			break;
		}
		wait(4);
	}

	printf("\n================================================================\n");
	printf("mcc_platform_init_connection\n");
	printf("\n================================================================\n");
	mcc_platform_init_connection();

	//printf("\n================================================================\n");
	//printf("attach\n");
	//printf("\n================================================================\n");
	//nw->attach(&nw_callback);

	//printf("\n================================================================\n");
	//printf("resume\n");
	//printf("\n================================================================\n");
	//mbedClient.resume(mcc_platform_get_network_interface());

	// close mbed-cloud-client
	//mbedClient.close();
	
    // Client unregistered, disconnect and exit program.
    printf("\n================================================================\n");
	printf("close network connection\n");
	printf("\n================================================================\n");
    mcc_platform_close_connection();
	#endif
}


//-------------------------------------------------------------------------------------//
// interface functions
//-------------------------------------------------------------------------------------//
void Pelion_ToDeregister(_DisconnectType DisconnectType)
{
	Main.DisconnectType = DisconnectType;
	Main.ToNetDisconnect = true;
}

void Pelion_ToRestart(void)
{
	Main.ToNetConnect = true;
}

bool Pelion_IsRegistered(void)
{
	return Main.PelionRegistered;
}

bool Pelion_IsPaused(void)
{
	return Main.Paused;
}

char *Pelion_GetDeviceID(void)
{
	return Main.DeviceIDStr;
}

_NetState Network_GetState(void)
{
	return Main.NetState;
}



//-------------------------------------------------------------------------------------//
// RTC functions
//-------------------------------------------------------------------------------------//
static int SyncRealTime(bool *terminate)
{
    printf("Sync time...\n");
	time_t timestamp;
	struct tm *t;
	NetworkInterface *net;
	net = (NetworkInterface *)mcc_platform_get_network_interface();
	if(!net)
	{
		printf("get net interface fail !!!\n");
		return -1;
	}

	int timeout = 3600;
	int retry = 0;
    NTPClient ntp(net);
    while(1) {
		if(terminate && *terminate)
		{
			printf("SyncRealTime: terminate\n");
			return 0;
		}
		
        timestamp = ntp.get_timestamp(timeout);
        if (timestamp < 0) {
            printf("SyncRealTime: An error occurred when getting the time. Code: %lld\n", timestamp);
			if(timestamp == NSAPI_ERROR_WOULD_BLOCK)
			{
				timeout += 200;
				if(timeout > 6000)
				{
					timeout = 6000;
					retry++;
				}
				printf("SyncRealTime: enlarge timeout time %d\n", timeout);
			}
			else if(timestamp == NSAPI_ERROR_NO_SOCKET)
			{
				printf("SyncRealTime: NSAPI_ERROR_NO_SOCKET %lld\n", timestamp);
				return NETWORK_ERROR_NO_SOCKET;
			}			
			else if(timestamp == NSAPI_ERROR_DEVICE_ERROR)
			{
				printf("SyncRealTime: NSAPI_ERROR_DEVICE_ERROR %lld\n", timestamp);
				return NETWORK_ERROR_DEVICE_ERROR;
			}
			else if(timestamp == NSAPI_ERROR_DNS_FAILURE)
			{
				printf("SyncRealTime: NSAPI_ERROR_DNS_FAILURE %lld\n", timestamp);
				retry++;
			}
			else
	            wait(2.0);

			if(retry >= 10)
			{
				printf("retry over 10 times and fail\n");
				return (int)timestamp;
			}
        } else {
            //printf("[MAIN] Current time is %s\n", ctime(&timestamp));
            t = localtime(&timestamp);
			if(IF_REAL_RTC(t->tm_year+NTP_DATETIME_BASE_YEAR))
			{
				printf("----- SyncRealTime time done at (internal) -----\n");
				RTC_PrintRealTime();
				#ifdef _TIMEZONE
				printf("timezone %c%d:%02d\n", (offsetHour>0)?'+':'-', offsetHour, offsetMinute);
				timestamp += (offsetHour*60*60 + offsetMinute*60);	// offset for timezone
				#endif
            	set_time(timestamp);
				
				printf("----- SyncRealTime time done at (real) -----\n");
				RTC_PrintRealTime();
            	break;
			}
			wait(2.0);
        }
    }
    timestamp = (time_t)time(NULL);
    t = localtime(&timestamp);
    char buffer[32];
    memset(buffer, 0, 32);
    strftime(buffer, 32, "%FT%T\n", t);
    for(int i=0; i<32; i++)
    {
        if( (buffer[i]=='\r') || (buffer[i]=='\n') )
            buffer[i] = 0;
    }
    printf("%s\n", buffer);
	printf("%d-%02d-%02d (w:%d) %02d:%02d:%02d (dst:%d)\n", t->tm_year+NTP_DATETIME_BASE_YEAR, t->tm_mon+NTP_DATETIME_BASE_MONTH, t->tm_mday, t->tm_wday,
													t->tm_hour, t->tm_min, t->tm_sec, t->tm_isdst);

	time_t unixtime = RTC_GetUnixTimeStamp();
	printf("timestamp     : %lld\n", RTC_GetRealTimeStamp());
	printf("unix-timestamp: %lld\n", unixtime);
	t = localtime(&unixtime);
	printf("unix-time: %d-%02d-%02d (w:%d) %02d:%02d:%02d (dst:%d)\n", t->tm_year, t->tm_mon, t->tm_mday, t->tm_wday,
													t->tm_hour, t->tm_min, t->tm_sec, t->tm_isdst);

	return 0;
}

void RTC_GetRealTime(_DateTimeStruct *rtcdatetime)
{
	time_t timestamp;
	struct tm *t;
	
	timestamp = (time_t)time(NULL);
	t = localtime(&timestamp);
	rtcdatetime->Y		= t->tm_year+NTP_DATETIME_BASE_YEAR;
	rtcdatetime->M		= t->tm_mon+NTP_DATETIME_BASE_MONTH;
	rtcdatetime->D		= t->tm_mday;
	rtcdatetime->WD		= t->tm_wday;
	rtcdatetime->h		= t->tm_hour;
	rtcdatetime->m		= t->tm_min;
	rtcdatetime->s		= t->tm_sec;
}

time_t RTC_GetRealTimeStamp(void)
{
	time_t timestamp;
	timestamp = (time_t)time(NULL);
	
	return timestamp;
}

time_t RTC_GetUnixTimeStamp(void)
{
	time_t timestamp;
	struct tm *t;
	
	timestamp = (time_t)time(NULL);
	t = localtime(&timestamp);
    t->tm_isdst = -1;					// Is Daylight saving time on? 1 = yes, 0 = no, -1 = unknown
 
    return mktime(t);					// returns seconds elapsed since January 1, 1970 (begin of the Epoch)
}

void RTC_TimeStampToRealTime(_DateTimeStruct *rtcdatetime, time_t timestamp)
{
	struct tm *t;
	
	t = localtime(&timestamp);
	rtcdatetime->Y		= t->tm_year+NTP_DATETIME_BASE_YEAR;
	rtcdatetime->M		= t->tm_mon+NTP_DATETIME_BASE_MONTH;
	rtcdatetime->D		= t->tm_mday;
	rtcdatetime->WD		= t->tm_wday;
	rtcdatetime->h		= t->tm_hour;
	rtcdatetime->m		= t->tm_min;
	rtcdatetime->s		= t->tm_sec;
}

void RTC_PrintRealTime(void)
{
	time_t timestamp;
	struct tm *t;
    char buffer[32];

    timestamp = (time_t)time(NULL);
    t = localtime(&timestamp);
    memset(buffer, 0, 32);
    strftime(buffer, 32, "%FT%T\n", t);
    for(int i=0; i<32; i++)
    {
        if( (buffer[i]=='\r') || (buffer[i]=='\n') )
            buffer[i] = 0;
    }
    printf("[%s]\n", buffer);
}


//-------------------------------------------------------------------------------------//
//	watchdog functions
//-------------------------------------------------------------------------------------//
static void Watchdog_InitStart(void)
{
	IwdgHandle.Instance			= IWDG;
	IwdgHandle.Init.Prescaler	= DEFAULT_WATCHDOG_PRESCALER;
	IwdgHandle.Init.Reload		= DEFAULT_WATCHDOG_COUNT;
	if(HAL_IWDG_Init(&IwdgHandle) == HAL_OK)
	{
		WatchdogTimer.start(DEFAULT_WATCHDOG_KICKPERIOD_SEC*1000);
		printf("launch watchdog ok\n");
	}
	else
		printf("launch watchdog fail\n");
}

static void Watchdog_Kick(void)
{
	__HAL_IWDG_RELOAD_COUNTER(&IwdgHandle);
}

static void Watchdog_TimerFxn(void const *n)
{
	Watchdog_Kick();
}

void OS_Reboot(_ModemResetType reset_modem_type)
{
	printf("********** OS_Reboot **********\n");

	switch(reset_modem_type)
	{
		case MODEM_RESET_NONE:
			printf("no reset modem\n");
			break;
		case MODEM_RESET_SW:
			printf("sw reset modem\n");
			CellularExt_SWReset_Reset();
			break;
		case MODEM_RESET_HW:
			printf("hw reset modem\n");
			CellularExt_HWReset_Reset();
			break;
	}
	
	printf("********** call os reboot api after 3 secs **********\n");
	wait(3);
	pal_osReboot();

	//printf("********** stop watchdog timer to trigger system reboot **********\n");
	//WatchdogTimer.stop();
}


//-------------------------------------------------------------------------------------//
//	Debug_PrintHeapStack
//-------------------------------------------------------------------------------------//
void Main_PrintHeapStack(const char *comment)
{
	#ifdef MBED_HEAP_STATS_ENABLED
	printf("---------------------------------------\n");
	printf("<heap> %s\r\n", comment);
	print_heap_stats();
	printf("---------------------------------------\n");
	#endif
	#ifdef MBED_STACK_STATS_ENABLED
	printf("---------------------------------------\n");
	printf("<stack> %s\r\n", comment);
	print_stack_statistics();
	printf("---------------------------------------\n");
	#endif
}

#endif // #ifdef _PROJECT_IZANAGI

