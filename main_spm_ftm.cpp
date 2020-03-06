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
#if 0
#include "common_def.h"

#ifdef _PROJECT_SPM_FTM

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
#include "mbed.h"
#include "application_init.h"
//#include "mcc_common_button_and_led.h"
//#include "blinky.h"

#include "common_functions.h"
#include "UDPSocket.h"

#include "nsapi_types.h"						// nsapi error code
#include "ntp-client/NTPClient.h"				// sync time
#include "CellularContext.h"					// CellularContext
#include "CellularNetwork.h"					// CellularNetwork
#include "CellularExternalControl.h"
#include "DataStore.h"

#include "common_debug.h"

#ifdef _DEBUG_MAIN
#define TAG         	"MAIN"
#define MAIN_DEBUG  	DEBUGMSG
#define MAIN_DUMP		DEBUGMSG_DUMP
#else
#define MAIN_DEBUG
#define MAIN_DUMP
#endif //_DEBUG_MAIN
#include "ADE7953.h"


//---------------------------------------------------------------------//
//	define constant
//---------------------------------------------------------------------//
#define FTM_TEST_RETRY_MAX			30


//---------------------------------------------------------------------//
//	function declaration
//---------------------------------------------------------------------//
static void main_application(void);
static bool main_testflash(void);
static bool main_netconnect(void);
static bool main_netdisconnect(void);
static bool main_echotest(void);
static bool main_echotest(void);
static bool main_rtctest(void);

extern int factory_flow_task();				// merge fce


//---------------------------------------------------------------------//
//	variables declaration
//---------------------------------------------------------------------//
typedef struct {
	// flash
	bool TestFlash;

	// rtc
	bool TestRtc;
} _MainStruct;

static _MainStruct Main;

static void nw_callback(nsapi_event_t ev, intptr_t intptr)
{
}

//---------------------------------------------------------------------//
//	functions
//---------------------------------------------------------------------//
int main(void)
{
    return mcc_platform_run_program(main_application);
}

void main_application(void)
{
	MAIN_DEBUG("================================================================");
	MAIN_DEBUG("FW Version: %d.%d", AAEON_FW_VERSION_MASTER, AAEON_FW_VERSION_SLAVE);
	MAIN_DEBUG("Date & Time: %s - %s", __DATE__, __TIME__);
	MAIN_DEBUG("================================================================");

	//---------------------------------------------------------------------//
    //  initialize variables
    //---------------------------------------------------------------------//
	int i, ret;
	Main.TestFlash				= false;
	Main.TestRtc				= false;
	
	#if defined(__linux__) && (MBED_CONF_MBED_TRACE_ENABLE == 0)
							 // make sure the line buffering is on as non-trace builds do
    // not produce enough output to fill the buffer
    setlinebuf(stdout);
	#endif

	// Initialize trace-library first
	if (application_init_mbed_trace() != 0) {
		printf("Failed initializing mbed trace\n" );
		return;
	}

	// Print platform information
	mcc_platform_sw_build_info();

	//---------------------------------------------------------------------//
	// Initialize storage
	//---------------------------------------------------------------------//
	if (mcc_platform_storage_init() != 0) {
		printf("Failed to initialize storage\n" );
		return;
	}

	//---------------------------------------------------------------------//
	// Initialize platform-specific components
	//---------------------------------------------------------------------//
	if(mcc_platform_init() != 0) {
		printf("ERROR - platform_init() failed!\n");
		return;
	}	

    // application_init() runs the following initializations:
    //  1. platform initialization
    //  2. print memory statistics if MBED_HEAP_STATS_ENABLED is defined
    //  3. FCC initialization.
	MAIN_DEBUG("================================================================");
	MAIN_DEBUG("initialize application");
	MAIN_DEBUG("================================================================");
    if (!application_init()) {
        printf("Initialization failed, exiting application!\n");
        return;
    }

	//---------------------------------------------------------------------//
	// fill test function in here
    //---------------------------------------------------------------------//
	ADE7953 ADETest;
	printf("initial %d\n", ADETest.init());
	while (1)
	{
		printf("=================================================\n");
		printf("%-15s 0x218 = %x\n", "InstVoltage", ADETest.getInstVoltage());
		printf("%-15s 0x21C = %x\n\n", "Vrms", ADETest.getVrms());
		printf("----------------Channel A------------------------\n");
		printf("%-15s 0x216 = %x\n", "InstCurrentA", ADETest.getInstCurrentA());
		printf("%-15s 0x21A = %x\n", "IrmsA", ADETest.getIrmsA());
		printf("%-15s 0x21E = %x\n", "ActiveEnergyA", ADETest.getActiveEnergyA());
		printf("%-15s 0x220 = %x\n", "ReactiveEnergyA", ADETest.getReactiveEnergyA());
		printf("%-15s 0x222 = %x\n\n", "ApparentEnergyA", ADETest.getApparentEnergyA());
		printf("-----------------Channel B-----------------------\n");
		printf("%-15s 0x217 = %x\n", "InstCurrentB", ADETest.getInstCurrentB());
		printf("%-15s 0x21B = %x\n", "IrmsB", ADETest.getIrmsB());
		printf("%-15s 0x21F = %x\n", "ActiveEnergyB", ADETest.getActiveEnergyB());
		printf("%-15s 0x221 = %x\n", "ReactiveEnergyB", ADETest.getReactiveEnergyB());
		printf("%-15s 0x223 = %x\n\n", "ApparentEnergyB", ADETest.getApparentEnergyB());
		printf("=================================================\n\n");
		wait(1);
	}

#if 0
    //---------------------------------------------------------------------//
	// external flash test
    //---------------------------------------------------------------------//
    printf("\n================================================================\n");
	printf("external flash test\n");
	printf("================================================================\n");
	Main.TestFlash = main_testflash();
	MAIN_DEBUG("TestFlash: %d", Main.TestFlash);

    //---------------------------------------------------------------------//
	// rtc test
    //---------------------------------------------------------------------//
	printf("\n================================================================\n");
	printf("rtc test\n");
	printf("================================================================\n");
	Main.TestRtc = main_rtctest();
	MAIN_DEBUG("TestRtc: %d", Main.TestRtc);
	
    //---------------------------------------------------------------------//
    //  Testing conclusion
    //---------------------------------------------------------------------//
	printf("\n================================================================\n");
	printf("SPI Flash Test: %s\n", (Main.TestFlash)? "PASS":"FAIL");
	printf("RTC Test: %s\n", (Main.TestRtc)? "PASS":"FAIL");
	printf("================================================================\n");

    //---------------------------------------------------------------------//
    //  factory flow
    //---------------------------------------------------------------------//
	#if MBED_CONF_APP_DEVELOPER_MODE != 1
	printf("\n================================================================\n");
	printf("FCU\n");
	printf("waiting inject certificate and related info...\n");
	printf("================================================================\n\n");
	ret = factory_flow_task(false);	// sophie, true: force to erase fce, ignore fcu if flag exist
	switch(ret)
	{
		case 0:
			printf("FCU SUCCESS and COMPLETE\n");
			break;
		case 1:
			printf("FCU has done before\n");
			break;
		default:
			printf("FCU FAIL %d\n", ret);
			break;
	}
	//wait(5);
	#endif
#endif
	printf("\nend and quit\n");

}

//-------------------------------------------------------------------------------------//
// flash functions
//-------------------------------------------------------------------------------------//
static bool main_testflash(void)
{
	bool ret;

	const char FTM_FLASH_TEST_FILE_NAME[]	= "ftm";
	uint8_t wbuff[30];
	uint8_t rbuff[30];
	int i, remain;

	ret = false;
	for(i=0; i<sizeof(wbuff); i++)
	{
		wbuff[i] = rand();
	}

	DataStore_Init(NULL);
	if(DataStore_WriteFile(FTM_FLASH_TEST_FILE_NAME, wbuff, sizeof(wbuff), false) > 0)
	{
		if(DataStore_ReadFile(FTM_FLASH_TEST_FILE_NAME, 0, rbuff, sizeof(rbuff), &remain) == sizeof(wbuff))
		{
			if(memcmp(wbuff, rbuff, sizeof(wbuff)) == 0)
			{
				ret = true;
			}
			else
			{
				MAIN_DEBUG("memcmp fail\n");
				//MAIN_DUMP("wbuff", i, sizeof(wbuff), wbuff);
				//MAIN_DUMP("rbuff", i, sizeof(rbuff), rbuff);
			}
		}
		else
			MAIN_DEBUG("DataStore_ReadFile fail\n");
		DataStore_RemoveFile(FTM_FLASH_TEST_FILE_NAME);
	}
	else
		MAIN_DEBUG("DataStore_WriteFile fail");

	return ret;
}

//-------------------------------------------------------------------------------------//
// RTC functions
//-------------------------------------------------------------------------------------//
static void get_time(_DateTimeStruct *rtcdatetime)
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

	MAIN_DEBUG("%d-%d-%d (%d) %d:%d:%d", rtcdatetime->Y, rtcdatetime->M, rtcdatetime->D, rtcdatetime->WD, rtcdatetime->h, rtcdatetime->m, rtcdatetime->s);
}

static bool main_rtctest(void)
{
	_DateTimeStruct rtcdatetime;

	get_time(&rtcdatetime);
    return true;
	//return IF_RTC(rtcdatetime.D);
}

#endif // #ifdef _PROJECT_SPM_FTM
#endif
