/*
 * Copyright (c) 2015-2018 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MCC_COMMON_SETUP_H
#define MCC_COMMON_SETUP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*main_t)(void);

// Initialize platform
// related platform specific initializations required.
//
// @returns
//   0 for success, anything else for error
int mcc_platform_init(void);

// Initialize network connection
int mcc_platform_init_connection(void);

// Close network connection
int mcc_platform_close_connection(void);

// Return network interface.
void *mcc_platform_get_network_interface(void);

//<sophie, define network init error code
// same as enum nsapi_error {} in \mbed-os\features\netsocket\nsapi_types.h
enum {
    NETWORK_ERROR_OK                  =  0,        /*!< no error */
    NETWORK_ERROR_WOULD_BLOCK         = -3001,     /*!< no data is not available but call is non-blocking */
    NETWORK_ERROR_UNSUPPORTED         = -3002,     /*!< unsupported functionality */
    NETWORK_ERROR_PARAMETER           = -3003,     /*!< invalid configuration */
    NETWORK_ERROR_NO_CONNECTION       = -3004,     /*!< not connected to a network */
    NETWORK_ERROR_NO_SOCKET           = -3005,     /*!< socket not available for use */
    NETWORK_ERROR_NO_ADDRESS          = -3006,     /*!< IP address is not known */
    NETWORK_ERROR_NO_MEMORY           = -3007,     /*!< memory resource not available */
    NETWORK_ERROR_NO_SSID             = -3008,     /*!< ssid not found */
    NETWORK_ERROR_DNS_FAILURE         = -3009,     /*!< DNS failed to complete successfully */
    NETWORK_ERROR_DHCP_FAILURE        = -3010,     /*!< DHCP failed to complete successfully */
    NETWORK_ERROR_AUTH_FAILURE        = -3011,     /*!< connection to access point failed */
    NETWORK_ERROR_DEVICE_ERROR        = -3012,     /*!< failure interfacing with the network processor */
    NETWORK_ERROR_IN_PROGRESS         = -3013,     /*!< operation (eg connect) in progress */
    NETWORK_ERROR_ALREADY             = -3014,     /*!< operation (eg connect) already in progress */
    NETWORK_ERROR_IS_CONNECTED        = -3015,     /*!< socket is already connected */
    NETWORK_ERROR_CONNECTION_LOST     = -3016,     /*!< connection lost */
    NETWORK_ERROR_CONNECTION_TIMEOUT  = -3017,     /*!< connection timed out */
    NETWORK_ERROR_ADDRESS_IN_USE      = -3018,     /*!< Address already in use */
    NETWORK_ERROR_TIMEOUT             = -3019,     /*!< operation timed out */

	// custom define for interface with main and mcc_platform_init_connection
	NETWORK_ERROR_NO_NETWORK_INTERFACE = -9999
};
//>sophie, define network init error code

// Return network status
//<sophie, add API to get network status
enum {
    NETWORK_STATUS_LOCAL_UP           = 0,        /*!< local IP address set */
    NETWORK_STATUS_GLOBAL_UP          = 1,        /*!< global IP address set */
    NETWORK_STATUS_DISCONNECTED       = 2,        /*!< no connection to network */
    NETWORK_STATUS_CONNECTING         = 3        /*!< connecting to network */
};
int mcc_platform_get_network_status(void);
//>sophie, add API to get network status

// Format storage (DEPRECATED)
int mcc_platform_reformat_storage(void);

// initialize common details for storage for storing KCM data etc.
// creates default folders, reformat.
int mcc_platform_storage_init(void);

// Wait
void mcc_platform_do_wait(int timeout_ms);

// for printing sW build info
void mcc_platform_sw_build_info(void);

/*!
 * @brief mcc_platform_run_program - Start the OS with the main function
 * @param testMain_t mainTestFunc  - main function to run
 * @return void
 */
int  mcc_platform_run_program(main_t mainFunc);
#ifdef __cplusplus
}
#endif

#endif // #ifndef MCC_COMMON_SETUP_H
