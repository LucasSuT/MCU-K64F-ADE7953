// /*
//  * Copyright (c) 2017 ARM Limited. All rights reserved.
//  * SPDX-License-Identifier: Apache-2.0
//  * Licensed under the Apache License, Version 2.0 (the License); you may
//  * not use this file except in compliance with the License.
//  * You may obtain a copy of the License at
//  *
//  * http://www.apache.org/licenses/LICENSE-2.0
//  *
//  * Unless required by applicable law or agreed to in writing, software
//  * distributed under the License is distributed on an AS IS BASIS, WITHOUT
//  * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
//  */
// #include "common_def.h"
// #define _ECHO_TEST_ENABLE 1
// #ifdef _ECHO_TEST_ENABLE

// #include "mbed.h"
// #include "common_functions.h"
// #include "UDPSocket.h"
// //#include "OnboardCellularInterface.h"
// //#include "EasyCellularConnection.h"
// //#include "mbed-trace/mbed_trace.h"
// #include "CellularLog.h"
// #include "CellularNonIPSocket.h"
// #include "CellularDevice.h"

// #define UDP 0
// #define TCP 1

// #define MBED_CONF_APP_SOCK_TYPE		TCP		// move from mbed_app.json in here

// // SIM pin code goes here
// /*#ifndef MBED_CONF_APP_SIM_PIN_CODE
// # define MBED_CONF_APP_SIM_PIN_CODE    "1234"
// #endif

// #ifndef MBED_CONF_APP_APN
// # define MBED_CONF_APP_APN         "internet"
// #endif
// #ifndef MBED_CONF_APP_USERNAME
// # define MBED_CONF_APP_USERNAME    NULL
// #endif
// #ifndef MBED_CONF_APP_PASSWORD
// # define MBED_CONF_APP_PASSWORD    NULL
// #endif
// */

// // Number of retries /
// #define RETRY_COUNT 5
// #define NUM_PACKETS 1 // sophie //50
// #define PACKET_INTERVAL 1 // 1sec // sophie //2 // 2 seconds



// // CellularInterface object
// //OnboardCellularInterface iface;
// //EasyCellularConnection iface;
// NetworkInterface *iface;

// // Echo server hostname
// // const char *host_name = "echo.mbedcloudtesting.com";
// // const char *host_name = "http://www.google.com";
// const char *host_name = "52.215.34.155";

//     // Echo server port (same for TCP and UDP)
//     const int port = 7;

// static rtos::Mutex trace_mutex;

// #if MBED_CONF_MBED_TRACE_ENABLE
// static void trace_wait()
// {
//    trace_mutex.lock();
// }

// static void trace_release()
// {
//    trace_mutex.unlock();
// }

// static char time_st[50];

// static char* trace_time(size_t ss)
// {
//    snprintf(time_st, 49, "[%08llums]", Kernel::get_ms_count());
//    return time_st;
// }

// static void trace_open()
// {
//    mbed_trace_init();
//    mbed_trace_prefix_function_set(&trace_time);
//    mbed_trace_mutex_wait_function_set(trace_wait);
//    mbed_trace_mutex_release_function_set(trace_release);
//    mbed_cellular_trace::mutex_wait_function_set(trace_wait);
//    mbed_cellular_trace::mutex_release_function_set(trace_release);
// }

// static void trace_close()
// {
//    mbed_cellular_trace::mutex_wait_function_set(NULL);
//    mbed_cellular_trace::mutex_release_function_set(NULL);
//    mbed_trace_free(); 
// }
// #endif // #if MBED_CONF_MBED_TRACE_ENABE

// Thread dot_thread(osPriorityNormal, 512);

// void print_function(const char *format, ...)
// {
//     trace_mutex.lock();
//     va_list arglist;
//     va_start (arglist, format);
//     vprintf(format, arglist);
//     va_end( arglist);
//     trace_mutex.unlock();
// }

// void dot_event()
// {

//     while (true) {
//         ThisThread::sleep_for(4000);
//         if (iface && iface->get_connection_status() == NSAPI_STATUS_GLOBAL_UP) {
//             break;
//         } else {
// 	    trace_mutex.lock();
//             printf(".");
//    	    fflush(stdout);
//   	    trace_mutex.unlock();
//         }
//     }
// }


// /**
//  * Connects to the Cellular Network
//  */
// nsapi_error_t do_connect()
// {
//     nsapi_error_t retcode = NSAPI_ERROR_OK;
//     uint8_t retry_counter = 0;

//     while (iface->get_connection_status() != NSAPI_STATUS_GLOBAL_UP) {

//         retcode = iface->connect();
//         if (retcode == NSAPI_ERROR_AUTH_FAILURE) {
//             print_function("\n\nAuthentication Failure. Exiting application\n");
//             return retcode;
//         } else if (retcode == NSAPI_ERROR_OK) {
//             print_function("\n\nConnection Established.\n");
//         } else if (retry_counter > RETRY_COUNT) {
//             print_function("\n\nFtal Connection failure: %d.\n", retcode);
//         //} else if (retcode != NSAPI_ERROR_OK && retry_counter > RETRY_COUNT) {        
// 		} else if (retcode != NSAPI_ERROR_OK && retry_counter < RETRY_COUNT) {
//             print_function("\n\nCouldn't connect: %d, will retry\n", retcode);
//             retry_counter++;
// 	    	continue;
//         }
//         break;
//     }

//     return retcode;
// }

// nsapi_error_t do_disconnect()
// {
//     nsapi_error_t retcode = NSAPI_ERROR_OK;
//     uint8_t retry_counter = 0;

//     while (iface->get_connection_status() != NSAPI_STATUS_DISCONNECTED) {

//         retcode = iface->disconnect();
//         if (retcode == NSAPI_ERROR_AUTH_FAILURE) {
//             print_function("\n\nAuthentication Failure. Exiting application\n");
//             return retcode;
//         } else if (retcode == NSAPI_ERROR_OK) {
//             print_function("\n\nConnection Terminated.\n");
//         } else if (retry_counter > RETRY_COUNT) {
//             print_function("\n\nFtal DisConnection failure: %d.\n", retcode);
//         //} else if (retcode != NSAPI_ERROR_OK && retry_counter > RETRY_COUNT) {
//         } else if (retcode != NSAPI_ERROR_OK && retry_counter < RETRY_COUNT) {
//              print_function("\n\nCouldn't disconnect: %d, will retry\n", retcode);
//             retry_counter++;
// 		    continue;
//         }
//         break;
//     }

//     return retcode;
// }





// /**
//  * Opens a UDP or a TCP socket with the given echo server and performs an echo
//  * transaction retrieving current.
//  */
// bool dnsfail = false;
// nsapi_error_t test_send_recv_tcp()
// {
//     nsapi_size_or_error_t retcode;
//     int i, j;
// 	unsigned int checksum_send, checksum_recv;
// #if MBED_CONF_APP_SOCK_TYPE == TCP
//     TCPSocket sock;
// #else
//     UDPSocket sock;
// #endif

// 	print_function("=================================================\n");
// 	print_function("test_send_recv_tcp\n");

//     retcode = sock.open(iface);
//     if (retcode != NSAPI_ERROR_OK) {
// #if MBED_CONF_APP_SOCK_TYPE == TCP
//         print_function("TCPSocket.open() fails, code: %d\n", retcode);
// #else
//         print_function("UDPSocket.open() fails, code: %d\n", retcode);
// #endif
//         return -1;
//     }

//     SocketAddress sock_addr;
//     retcode = iface->gethostbyname(host_name, &sock_addr);
//     if (retcode != NSAPI_ERROR_OK) {
//         print_function("Couldn't resolve remote host: %s, code: %d\n", host_name, retcode);
//         //return -1; // sophie
//         // dnsfail = true;
// 		//sock_addr.set_ip_address("195.34.89.241");
// 		sock_addr.set_ip_address("52.215.34.155");
//     }
// 	print_function("=================================================\n");

// 	//sock_addr.set_ip_address("195.34.89.241"); // test
	
//     sock_addr.set_port(port);
// #if 1
//     sock.set_timeout(15000);
//     //const char *echo_string = "TEST"; // sophie test
//     #define TEST_LENGTH		1024
//     uint8_t echo_string[TEST_LENGTH];
//     uint8_t recv_buf[TEST_LENGTH];

// 	for(i=0; i<TEST_LENGTH; i++)
// 		echo_string[i] = (uint8_t)i;
	
// #if MBED_CONF_APP_SOCK_TYPE == TCP
//     retcode = sock.connect(sock_addr);
//     if (retcode < 0) {
//         print_function("TCPSocket.connect() fails, code: %d\n", retcode);
//         return -1;
//     } else {
//         print_function("TCP: connected with %s server\n", sock_addr.get_ip_address()); //host_name);
//     }
// #endif

//     i = 0;
//     while (i < NUM_PACKETS) { // send NUM_PACKETS packets
// 		print_function("\n\n=================================================\n");
// 		print_function("<%d>\n", i);
// 		print_function("=================================================\n");

// 		memset(echo_string, i, 5);
// 		memset(recv_buf, 0, TEST_LENGTH);

// 		checksum_send = 0;
// 		checksum_recv = 0;
		
// 		#if MBED_CONF_APP_SOCK_TYPE == TCP
// 	    retcode = sock.send((void*) echo_string, sizeof(echo_string));
// 	    if (retcode < 0) {
// 	        print_function("TCPSocket.send() fails, code: %d\n", retcode);
// 	        // return -1;  // Don't exit. Continue with the next packet
// 	    } else {
// 	        print_function("TCP: Sent %d Bytes to %s\n", retcode, sock_addr.get_ip_address()); //host_name);
// 	    }

// 		j = 0; // sophie
// 		retcode = 0;
// 		while(j++ < RETRY_COUNT)
// 		{
// 			print_function("-------------- recv %d\n", j-1);
// 	    	retcode += sock.recv((void*) recv_buf+retcode, sizeof(recv_buf)-retcode);
// 	    	if (retcode > 0) {
// 	    		print_function("TCP: Received %d Bytes %s from %s tries: %d\n", retcode, recv_buf, sock_addr.get_ip_address()/*host_name*/, i + 1);
// 				if(retcode >= sizeof(echo_string))
// 					break;
// 			} else {
// 	    	     print_function("TCPSocket.recvfrom() fails after %d tries, code: %d\n", i + 1, retcode);
// 	    	}
// 		}

// 		#else // #if MBED_CONF_APP_SOCK_TYPE == TCP

// 	    retcode = sock.sendto(sock_addr, (void*) echo_string, sizeof(echo_string));
// 	    if (retcode < 0) {
// 	        print_function("UDPSocket.sendto() fails, pkt #: %d, code: %d\n", i+1, retcode);
// 	        // return -1; // Don't exit. Continue with the next package
// 	    } else {
// 	        print_function("UDP: Sent %d Bytes to %s\n", retcode, sock_addr.get_ip_address()); //host_name);
// 	    }

// 		j = 0; // sophie
// 		while(j++ < RETRY_COUNT)
// 		{
// 			print_function("-------------- recv %d\n", j-1);
// 	    	retcode = sock.recvfrom(&sock_addr, (void*) recv_buf, sizeof(recv_buf));
// 	    	if (retcode > 0) {
// 	    	    print_function("UDP: Received %d Bytes %s from %s tries: %d\n", retcode, recv_buf, sock_addr.get_ip_address()/*host_name*/, i + 1);
// 	    		break;
// 			} else {
// 	    	    print_function("UDPSocket.recvfrom() fails after %d tries, code: %d\n", i + 1, retcode);
// 	    	}
// 		}
// 		#endif

// 		if(retcode > 0)
// 		{
// 			for(j=0; j<TEST_LENGTH; j++)
// 			{
// 				checksum_send += echo_string[j];
// 				checksum_recv += recv_buf[j];
// 			}
// 			print_function("\n>>>>> Verify checksum %s\n\n", (checksum_send==checksum_recv)? "PASS" : "FAIL");
// 		}
				
//         i++;
//         wait(PACKET_INTERVAL); // wait PACKET_INTERVAL seconds before sending next packet 
//     } 
// 	print_function("=================================================\n");
//     #endif
//     sock.close();
//     return 0;
// }

// nsapi_error_t test_send_recv_udp()
// {
//     nsapi_size_or_error_t retcode;
//     int i, j;
// 	unsigned int checksum_send, checksum_recv;
//     UDPSocket sock;

// 	print_function("=================================================\n");
// 	print_function("test_send_recv_udp\n");

//     retcode = sock.open(iface);
//     if (retcode != NSAPI_ERROR_OK) {
//         print_function("UDPSocket.open() fails, code: %d\n", retcode);
//         return -1;
//     }

//     SocketAddress sock_addr;
//     retcode = iface->gethostbyname(host_name, &sock_addr);
//     if (retcode != NSAPI_ERROR_OK) {
//         print_function("Couldn't resolve remote host: %s, code: %d\n", host_name, retcode);
//         //return -1; // sophie
//         dnsfail = true;
// 		//sock_addr.set_ip_address("195.34.89.241");
// 		sock_addr.set_ip_address("52.215.34.155");
//     }
// 	print_function("=================================================\n");

// 	//sock_addr.set_ip_address("195.34.89.241"); // test
	
//     sock_addr.set_port(port);

//     sock.set_timeout(15000);
//     //const char *echo_string = "TEST"; // sophie test
//     #define TEST_LENGTH		1024
//     uint8_t echo_string[TEST_LENGTH];
//     uint8_t recv_buf[TEST_LENGTH];

// 	for(i=0; i<TEST_LENGTH; i++)
// 		echo_string[i] = (uint8_t)i;
	
//     i = 0;
//     while (i < NUM_PACKETS) { // send NUM_PACKETS packets
// 		print_function("\n\n=================================================\n");
// 		print_function("<%d>\n", i);
// 		print_function("=================================================\n");

// 		memset(echo_string, i, 5);
// 		memset(recv_buf, 0, TEST_LENGTH);

// 		checksum_send = 0;
// 		checksum_recv = 0;
		
// 	    retcode = sock.sendto(sock_addr, (void*) echo_string, sizeof(echo_string));
// 	    if (retcode < 0) {
// 	        print_function("UDPSocket.sendto() fails, pkt #: %d, code: %d\n", i+1, retcode);
// 	        // return -1; // Don't exit. Continue with the next package
// 	    } else {
// 	        print_function("UDP: Sent %d Bytes to %s\n", retcode, sock_addr.get_ip_address()); //host_name);
// 	    }

// 		j = 0; // sophie
// 		while(j++ < RETRY_COUNT)
// 		{
// 			print_function("-------------- recv %d\n", j-1);
// 	    	retcode = sock.recvfrom(&sock_addr, (void*) recv_buf, sizeof(recv_buf));
// 	    	if (retcode > 0) {
// 	    	    print_function("UDP: Received %d Bytes %s from %s tries: %d\n", retcode, recv_buf, sock_addr.get_ip_address()/*host_name*/, i + 1);
// 	    		break;
// 			} else {
// 	    	    print_function("UDPSocket.recvfrom() fails after %d tries, code: %d\n", i + 1, retcode);
// 	    	}
// 		}

// 		if(retcode > 0)
// 		{
// 			for(j=0; j<TEST_LENGTH; j++)
// 			{
// 				checksum_send += echo_string[j];
// 				checksum_recv += recv_buf[j];
// 			}
// 			print_function("\n>>>>> Verify checksum %s\n\n", (checksum_send==checksum_recv)? "PASS" : "FAIL");
// 		}
				
//         i++;
//         wait(PACKET_INTERVAL); // wait PACKET_INTERVAL seconds before sending next packet 
//     } 
// 	print_function("=================================================\n");
//     sock.close();
//     return 0;
// }

// /*nsapi_error_t AT_CellularInformation::get_info(const char *cmd, char *buf, size_t buf_size)
// {
//     _at.lock();

//     _at.cmd_start(cmd);
//     _at.cmd_stop();
//     _at.set_delimiter(0);
//     _at.resp_start();
//     _at.read_string(buf, buf_size-1);
//     _at.resp_stop();
//     _at.set_default_delimiter();

//     return _at.unlock_return_error();
// }
// void get_modem_info()
// {
//     UARTSerial *uart_ptr = iface.get_serial();
//     ATHandler *atHandler;
//     char rsp_buf[100];
//     int rsp_buf_len = 100;

//     if (uart_ptr) {
//         atHandler = get_at_handler((FileHandle *) uart_ptr);
//         if (ATHandler) {
//             atHandler->lock();
//             atHandler->cmd_start("AT+CGMR");
//             atHandler->cmd_stop();
//             atHandler->set_delimiter(0);
//             atHandler->resp_start();
//             atHandler->read_string(rsp_buf, (rsp_buf_len -1));
//             atHandler->resp_stop();
//             atHandler->set_default_delimiter();
//             atHandler->unlock_return_error();    
//         }
//     }
// }*/
// /*
// void get_modem_info()
// {
//     CellularInformation *modem_info = iface.get_device()->open_information(iface.get_serial());
//     char rsp_buf[200]="";
//     int rsp_buf_len = 200;

//     if (modem_info)
//     {
//         modem_info->get_revision(rsp_buf, (rsp_buf_len -1));
//         snprintf(print_text, PRINT_TEXT_LENGTH, "manufacture info = %s\n", rsp_buf);
//         print_function(print_text);
//     } else {
//         print_function("modem_info is NULL !!!!\n");
//     }
// }
// */
// #include "CellularExternalControl.h"
// #if 0
// int main()
// {
//     #if defined(_DEVICE_QUECTEL_BG96) || defined(_DEVICE_UBLOX_SARA_R410M)
//     CellularExt_HWReset_Set(1); // for bg96 and ublox sara-n410, default high
//     #endif

//     nsapi_error_t retcode; 
//     /* initialized trace log */
//     //mbed_trace_init();
    
//     //iface.modem_debug_on(MBED_CONF_APP_MODEM_TRACE);
//     /* Set Pin code for SIM card */
//     //iface.set_sim_pin(MBED_CONF_APP_SIM_PIN_CODE);

//     /* Set network credentials here, e.g., APN*/
//     //iface.set_credentials(MBED_CONF_APP_APN, MBED_CONF_APP_USERNAME, MBED_CONF_APP_PASSWORD);


//     print_function("\n\nmbed-os-example-cellular\n");
//     print_function("Establishing connection\n");

// #if MBED_CONF_MBED_TRACE_ENABLE
//     trace_open();
// #else
//     dot_thread.start(dot_event);
// #endif

//     iface = NetworkInterface::get_default_instance();
//     MBED_ASSERT(iface);
//     retcode = NSAPI_ERROR_NO_CONNECTION;

// 	CellularExt_PreInitModule();
	
// 	while(1)	// loop test for CHT-network testing
// 	{
//         print_function("Doing atttach\n");
//         CellularExt_Attach();
//         /* Attempt to connect to a cellular network */
//         print_function("do_connect\n");
// 	    if ( (retcode=do_connect()) == NSAPI_ERROR_OK) {
// 	        //get_modem_info();

// 	        // print_function(">>>>> test_send_recv_udp\n");
// 	        // retcode = test_send_recv_udp();

//             // print_function(">>>>> test_send_recv_tcp\n");
// 	        // retcode = test_send_recv_tcp();

//             printf("wait for 45 secs\n");
// 			wait(10);
// 	    }

// 	    if (iface->disconnect() != NSAPI_ERROR_OK) {
// 		print_function("\n\n disconnect failed.\n\n");
// 	    }

// 	    if (retcode == NSAPI_ERROR_OK) {
// 		print_function("\n\nSuccess.\n\n");
// 	    } else {
// 		print_function("\n\nFailure.\n\n");
// 	    }

// 		/* Attempt to disconnect to a cellular network */
// 		print_function("do_disconnect\n");
// 		if (do_disconnect() == NSAPI_ERROR_OK) {
//             print_function("\n\n disconnect pass.\n\n"); //Lu
//         }		

// 		print_function("Doing detach\n");
// 		CellularExt_Detach();

// 		print_function("Wait for 15 secs\n");
// 		wait(15);
// 		// print_function("Doing atttach\n");
// 		// CellularExt_Attach();
// 	}

// #if MBED_CONF_MBED_TRACE_ENABLE
//     trace_close();
// #else
//     dot_thread.terminate();
// #endif

//     return 0;
// }
// // EOF
// #endif



// /**
//  * Opens:
//  * - UDP or TCP socket with the given echo server and performs an echo
//  *   transaction retrieving current.
//  * - Cellular Non-IP socket for which the data delivery path is decided
//  *   by network's control plane CIoT optimisation setup, for the given APN.
//  */
// nsapi_error_t test_send_recv()
// {
//     nsapi_size_or_error_t retcode;
// #if MBED_CONF_APP_SOCK_TYPE == TCP
//     TCPSocket sock;
// #elif MBED_CONF_APP_SOCK_TYPE == UDP
//     UDPSocket sock;
// #elif MBED_CONF_APP_SOCK_TYPE == NONIP
//     CellularNonIPSocket sock;
// #endif

// #if MBED_CONF_APP_SOCK_TYPE == NONIP
//     retcode = sock.open((CellularContext *)iface);
// #else
//     retcode = sock.open(iface);
// #endif

//     if (retcode != NSAPI_ERROR_OK)
//     {
// #if MBED_CONF_APP_SOCK_TYPE == TCP
//         print_function("TCPSocket.open() fails, code: %d\n", retcode);
// #elif MBED_CONF_APP_SOCK_TYPE == UDP
//         print_function("UDPSocket.open() fails, code: %d\n", retcode);
// #elif MBED_CONF_APP_SOCK_TYPE == NONIP
//         print_function("CellularNonIPSocket.open() fails, code: %d\n", retcode);
// #endif
//         return -1;
//     }

//     int n = 0;
//     const char *echo_string = "TEST";
//     char recv_buf[4];

//     sock.set_timeout(15000);

// #if MBED_CONF_APP_SOCK_TYPE == NONIP
//     retcode = sock.send((void *)echo_string, strlen(echo_string));
//     if (retcode < 0)
//     {
//         print_function("CellularNonIPSocket.send() fails, code: %d\n", retcode);
//         return -1;
//     }
//     else
//     {
//         print_function("CellularNonIPSocket: Sent %d Bytes\n", retcode);
//     }

//     n = sock.recv((void *)recv_buf, sizeof(recv_buf));

// #else

//     SocketAddress sock_addr;
//     retcode = iface->gethostbyname(host_name, &sock_addr);
//     // print_function("\n\n gethostAddr= %s\n\n", sock_addr.get_ip_address());
//     // print_function("\n\n gethostNoIPAddr= %s\n\n", sock_addr.get_addr());
//     // retcode = sock_addr.set_ip_address("52.215.34.155");
//     if (retcode != NSAPI_ERROR_OK)
//     {
//         print_function("Couldn't resolve remote host: %s, code: %d\n", host_name, retcode);
//         // return -1;
//         sock_addr.set_ip_address("52.215.34.155");
//         print_function("\n\n changeHostAddr= %s\n\n", sock_addr.get_ip_address());
//     }

//     sock_addr.set_port(port);

// #if MBED_CONF_APP_SOCK_TYPE == TCP
//     retcode = sock.connect(sock_addr);
//     if (retcode < 0)
//     {
//         print_function("TCPSocket.connect() fails, code: %d\n", retcode);
//         return -1;
//     }
//     else
//     {
//         print_function("TCP: connected with %s server\n", host_name);
//     }
//     retcode = sock.send((void *)echo_string, strlen(echo_string));
//     if (retcode < 0)
//     {
//         print_function("TCPSocket.send() fails, code: %d\n", retcode);
//         return -1;
//     }
//     else
//     {
//         print_function("TCP: Sent %d Bytes to %s\n", retcode, host_name);
//     }

//     n = sock.recv((void *)recv_buf, sizeof(recv_buf));
// #else

//     retcode = sock.sendto(sock_addr, (void *)echo_string, strlen(echo_string));
//     if (retcode < 0)
//     {
//         print_function("UDPSocket.sendto() fails, code: %d\n", retcode);
//         return -1;
//     }
//     else
//     {
//         print_function("UDP: Sent %d Bytes to %s\n", retcode, host_name);
//     }

//     n = sock.recvfrom(&sock_addr, (void *)recv_buf, sizeof(recv_buf));
// #endif
// #endif

//     sock.close();

//     if (n > 0)
//     {
//         print_function("Received from echo server %d Bytes\n", n);
//         return 0;
//     }

//     return -1;
// }

// #if 1
// int main()
// {
//     CellularExt_HWReset_Set(1);
//     print_function("\n\nmbed-os-example-cellular\n");
//     print_function("\n\nBuilt: %s, %s\n", __DATE__, __TIME__);
// #ifdef MBED_CONF_NSAPI_DEFAULT_CELLULAR_PLMN
//     print_function("\n\n[MAIN], plmn: %s\n", (MBED_CONF_NSAPI_DEFAULT_CELLULAR_PLMN ? MBED_CONF_NSAPI_DEFAULT_CELLULAR_PLMN : "NULL"));
// #endif

//     print_function("Establishing connection\n");
// #if MBED_CONF_MBED_TRACE_ENABLE
//     trace_open();
// #else
//     dot_thread.start(dot_event);
// #endif // #if MBED_CONF_MBED_TRACE_ENABLE

// #if MBED_CONF_APP_SOCK_TYPE == NONIP
//     iface = CellularContext::get_default_nonip_instance();
// #else
//     iface = CellularContext::get_default_instance();
// #endif

//     MBED_ASSERT(iface);
    
//     // sim pin, apn, credentials and possible plmn are taken automatically from json when using NetworkInterface::set_default_parameters()
//     // iface->set_default_parameters();

//     nsapi_error_t retcode = NSAPI_ERROR_NO_CONNECTION;
//     CellularExt_PreInitModule();
//     // print_function("Doing atttach\n");
//     // CellularExt_Attach();
//     /* Attempt to connect to a cellular network */
//     while(1){
//     if (do_connect() == NSAPI_ERROR_OK)
//     {
//         print_function("\n\n connect pass.\nwait 5 second.\n\n");
//         wait(5);
//         retcode = test_send_recv();
//         print_function("\n\n connect= %d.\n\n",retcode);
//     }

//     if (iface->disconnect() != NSAPI_ERROR_OK)
//     {
//         print_function("\n\n disconnect failed.\n\n");
//     }

//     if (retcode == NSAPI_ERROR_OK)
//     {
//         print_function("\n\nSuccess. Exiting \n\n");
//     }
//     else
//     {
//         print_function("\n\nFailure. Exiting \n\n");
//     }
//     print_function("\n\n nwait 5 second.\n\n");
//     wait(5);
//     }

// #if MBED_CONF_MBED_TRACE_ENABLE
//     trace_close();
// #else
//     dot_thread.terminate();
// #endif // #if MBED_CONF_MBED_TRACE_ENABLE

//     return 0;
// }
// #endif
// #endif // #ifdef _ECHO_TEST_ENABLE
