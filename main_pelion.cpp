// // ----------------------------------------------------------------------------
// // Copyright 2016-2018 ARM Ltd.
// //
// // SPDX-License-Identifier: Apache-2.0
// //
// // Licensed under the Apache License, Version 2.0 (the "License");
// // you may not use this file except in compliance with the License.
// // You may obtain a copy of the License at
// //
// //     http://www.apache.org/licenses/LICENSE-2.0
// //
// // Unless required by applicable law or agreed to in writing, software
// // distributed under the License is distributed on an "AS IS" BASIS,
// // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// // See the License for the specific language governing permissions and
// // limitations under the License.
// // ----------------------------------------------------------------------------
// #ifndef MBED_TEST_MODE

// #include "mbed.h"
// #include "simple-mbed-cloud-client.h"
// #include "FATFileSystem.h"
// #include "LittleFileSystem.h"
// #include "common_def.h"
// #include "common_debug.h"

// // Default network interface object. Don't forget to change the WiFi SSID/password in mbed_app.json if you're using WiFi.
// NetworkInterface *net2 = NetworkInterface::get_default_instance();

// // Default block device available on the target board
// BlockDevice *bd2 = BlockDevice::get_default_instance();

// #if COMPONENT_SD || COMPONENT_NUSD
// // Use FATFileSystem for SD card type blockdevices
// FATFileSystem fs2("fs");
// #else
// // Use LittleFileSystem for non-SD block devices to enable wear leveling and other functions
// LittleFileSystem fs2("fs");
// #endif

// // Default User button for GET example
// InterruptIn button(BUTTON1);
// // Default LED to use for PUT/POST example
// DigitalOut led(LED1, 0);

// // How often to fetch sensor data (in seconds)
// #define SENSORS_POLL_INTERVAL 3.0

// // Send all sensor data or just limited (useful for when running out of memory)
// #define SEND_ALL_SENSORS

// // Sensors related includes and initialization
// // Temperature reading from microcontroller
// AnalogIn adc_temp(ADC_TEMP);
// // Voltage reference reading from microcontroller
// AnalogIn adc_vref(ADC_VREF);

// // Declaring pointers for access to Pelion Device Management Client resources outside of main()
// MbedCloudClientResource *res_button;
// MbedCloudClientResource *res_led;
// MbedCloudClientResource *res_post;

// // Additional resources for sensor readings
// #ifdef SEND_ALL_SENSORS
// MbedCloudClientResource *res_temperature;
// MbedCloudClientResource *res_voltage;
// #endif /* SEND_ALL_SENSORS */

// // An event queue is a very useful structure to debounce information between contexts (e.g. ISR and normal threads)
// // This is great because things such as network operations are illegal in ISR, so updating a resource in a button's fall() function is not allowed
// EventQueue eventQueue;

// // When the device is registered, this variable will be used to access various useful information, like device ID etc.
// static const ConnectorClientEndpointInfo* endpointInfo;

// /**
//  * PUT handler - sets the value of the built-in LED
//  * @param resource The resource that triggered the callback
//  * @param newValue Updated value for the resource
//  */
// void put_callback(MbedCloudClientResource *resource, m2m::String newValue) {
//     printf("PUT received. New value: %s\n", newValue.c_str());
//     led = atoi(newValue.c_str());
// }

// /**
//  * POST handler - prints the content of the payload
//  * @param resource The resource that triggered the callback
//  * @param buffer If a body was passed to the POST function, this contains the data.
//  *               Note that the buffer is deallocated after leaving this function, so copy it if you need it longer.
//  * @param size Size of the body
//  */
// void post_callback(MbedCloudClientResource *resource, const uint8_t *buffer, uint16_t size) {
//     printf("POST received (length %u). Payload: ", size);
//     for (size_t ix = 0; ix < size; ix++) {
//         printf("%02x ", buffer[ix]);
//     }
//     printf("\n");
// }

// /**
//  * Button handler
//  * This function will be triggered either by a physical button press or by a ticker every 5 seconds (see below)
//  */
// void button_press() {
//     int v = res_button->get_value_int() + 1;
//     res_button->set_value(v);
//     printf("Button clicked %d times\n", v);
// }

// /**
//  * Notification callback handler
//  * @param resource The resource that triggered the callback
//  * @param status The delivery status of the notification
//  */
// void button_callback(MbedCloudClientResource *resource, const NoticationDeliveryStatus status) {
//     printf("Button notification, status %s (%d)\n", MbedCloudClientResource::delivery_status_to_string(status), status);
// }

// /**
//  * Registration callback handler
//  * @param endpoint Information about the registered endpoint such as the name (so you can find it back in portal)
//  * When the device is registered, this variable will be used to access various useful information, like device ID etc.
//  */
// void registered(const ConnectorClientEndpointInfo *endpoint) {
//     printf("Registered to Pelion Device Management. Endpoint Name: %s\n", endpoint->internal_endpoint_name.c_str());
//     endpointInfo = endpoint;
// }

// /**
//  * Update sensors and report their values.
//  * This function is called periodically.
//  */
// void sensors_update() {
//     float temp = adc_temp.read()*100;
//     float vref = adc_vref.read();
//     printf("ADC temp:  %6.4f C,  vref: %6.4f %%\r\n", temp, vref);
//     if (endpointInfo) {
//         res_temperature->set_value(temp);
//         res_voltage->set_value(vref);
//     }
// }

// #if 0
// int main(void) {
//     printf("\nStarting Simple Pelion Device Management Client example\n");

//     int storage_status = fs2.mount(bd2);
//     if (storage_status != 0) {
//         printf("Storage mounting failed.\n");
//     }
//     // If the User button is pressed ons start, then format storage.
//     // bool btn_pressed = (button.read() == MBED_CONF_APP_BUTTON_PRESSED_STATE);
//     // if (btn_pressed) {
//     //     printf("User button is pushed on start...\n");
//     // }

//     if (1 ) {
//         printf("Formatting the storage...\n");
//         int storage_status = StorageHelper::format(&fs2, bd2);
//         if (storage_status != 0) {
//             printf("ERROR: Failed to reformat the storage (%d).\n", storage_status);
//         }
//     } else {
//         printf("You can hold the user button during boot to format the storage and change the device identity.\n");
//     }

//     // Connect to the Internet (DHCP is expected to be on)
//     printf("Connecting to the network using the default network interface...\n");
//     net2 = NetworkInterface::get_default_instance();

//     nsapi_error_t net_status = NSAPI_ERROR_NO_CONNECTION;
// #include "CellularExternalControl.h"
//     CellularExt_HWReset_Set(1);
//     CellularExt_PreInitModule();
//     printf("Doing atttach\n");
//     CellularExt_Attach();
//     while ((net_status = net2->connect()) != NSAPI_ERROR_OK) {
//         printf("Unable to connect to network (%d). Retrying...\n", net_status);
//     }

//     printf("Connected to the network successfully. IP address: %s\n", net2->get_ip_address());

//     printf("Initializing Pelion Device Management Client...\n");

//     // SimpleMbedCloudClient handles registering over LwM2M to Pelion Device Management
//     SimpleMbedCloudClient client(net2, bd2, &fs2);
//     // int client_status = client.init();
//     int client_status = 0;
//     if (client_status != 0) {
//         printf("\nPelion Client initialization failed (%d)\n", client_status);
//         return -1;
//     }
//     else printf("\nPelion Client initialization passed (%d)\n", client_status);

//     // Creating resources, which can be written or read from the cloud
//     res_button = client.create_resource("3200/0/5501", "Button Count");
//     res_button->set_value(0);
//     res_button->methods(M2MMethod::GET);
//     res_button->observable(true);
//     res_button->attach_notification_callback(button_callback);

//     res_led = client.create_resource("3201/0/5853", "LED State");
//     res_led->set_value(led.read());
//     res_led->methods(M2MMethod::GET | M2MMethod::PUT);
//     res_led->attach_put_callback(put_callback);

//     res_post = client.create_resource("3300/0/5605", "Execute Function");
//     res_post->methods(M2MMethod::POST);
//     res_post->attach_post_callback(post_callback);

//     // Sensor resources
//     res_temperature = client.create_resource("3303/0/5700", "Temperature (C)");
//     res_temperature->set_value(0);
//     res_temperature->methods(M2MMethod::GET);
//     res_temperature->observable(true);

//     res_voltage = client.create_resource("3316/0/5700", "Voltage");
//     res_voltage->set_value(0);
//     res_voltage->methods(M2MMethod::GET);
//     res_voltage->observable(true);

//     printf("Initialized Pelion Device Management Client. Registering...\n");

//     // Callback that fires when registering is complete
//     client.on_registered(&registered);

//     // Register with Pelion DM
//     client.register_and_connect();

//     // The button fires on an interrupt context, but debounces it to the eventqueue, so it's safe to do network operations
//     button.fall(eventQueue.event(&button_press));
//     printf("Press the user button to increment the LwM2M resource value...\n");

//     Ticker timer;
//     timer.attach(eventQueue.event(&sensors_update), SENSORS_POLL_INTERVAL);

//     // You can easily run the eventQueue in a separate thread if required
//     eventQueue.dispatch_forever();
// }
// #endif

// #endif /* MBED_TEST_MODE */

// #if 0

// int main()
// {

//     /* Initialize connectivity */
//     NetworkInterface *net = NetworkInterface::get_default_instance();
//     #include "CellularExternalControl.h"
//         CellularExt_HWReset_Set(1);
//         CellularExt_PreInitModule();
//         net->connect();

//         /* Initialize storage */
//         BlockDevice *bd = BlockDevice::get_default_instance();

// #if COMPONENT_SD || COMPONENT_NUSD
//             // Use FATFileSystem for SD card type blockdevices
//             FATFileSystem fs("fs");
//     #else
//             // Use LittleFileSystem for non-SD block devices to enable wear leveling and other functions
//             LittleFileSystem fs("fs");
//     #endif

//     printf("\n\nstep1 success.\n\n");
//     /* Initialize SimpleMbedCloudClient */
//     SimpleMbedCloudClient client(net, bd, &fs);
//     printf("\n\nstep2 success.\n\n");
//     client.init();
//     printf("\n\nstep3 success.\n\n");
//     /* Create resource */
//     MbedCloudClientResource *variable;
//     variable = client.create_resource("3201/0/5853", "variable");
//     // variable->set_value("assign new value");
//     // variable->methods(M2MMethod::GET | M2MMethod::PUT);
//     // printf("\n\nstep4 over.\n\n");
// }
// #endif