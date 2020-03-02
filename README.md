The full documentation for this example is [available on our documentation site](https://cloud.mbed.com/docs/current/connecting/device-management-client-tutorials.html)



to replace to different cellular module, you need to modify some config as below:

//-------------------------------------------//
// Ublox R410M
//-------------------------------------------//
1. \mbed_app.json
    "macros": [
        "CELLULAR_DEVICE=UBLOX_AT",
        "UBLOX_SARA_R410M=1",
	"BAK__PAL_UDP_MTU_SIZE=1358",

2. \mbed-os\features\cellular\framework\targets\UBLOX\AT\mbed_lib.json
        "provide-default": {
            "help": "Provide as default CellularDevice [true/false]",
            "value": true
        }
   \mbed-os\features\cellular\framework\targets\QUECTEL\BG96\mbed_lib.json
   \mbed-os\features\cellular\framework\targets\QUECTEL\BC95\mbed_lib.json
        "provide-default": {
            "help": "Provide as default CellularDevice [true/false]",
            "value": false
        }

3. \common_def.h
	#define _DEVICE_UBLOX_SARA_R410M
	#define x_DEVICE_QUECTEL_BG96
	#define x_DEVICE_QUECTEL_BC95

//-------------------------------------------//
// Quectel BG96
//-------------------------------------------//
1. \mbed_app.json
    "macros": [
        "CELLULAR_DEVICE=QUECTEL_BG96",
        "PAL_UDP_MTU_SIZE=1358",
        "BAK__UBLOX_SARA_R410M=1",

2. \mbed-os\features\cellular\framework\targets\QUECTEL\BG96\mbed_lib.json
        "provide-default": {
            "help": "Provide as default CellularDevice [true/false]",
            "value": true
        }
   \mbed-os\features\cellular\framework\targets\UBLOX\AT\mbed_lib.json
   \mbed-os\features\cellular\framework\targets\QUECTEL\BC95\mbed_lib.json
        "provide-default": {
            "help": "Provide as default CellularDevice [true/false]",
            "value": false
        }

3. \common_def.h
	#define _DEVICE_QUECTEL_BG96
	#define x_DEVICE_UBLOX_SARA_R410M
	#define x_DEVICE_QUECTEL_BC95

//-------------------------------------------//
// Quectel BG96
//-------------------------------------------//
1. \mbed_app.json
    "macros": [
        "CELLULAR_DEVICE=QUECTEL_BC95",
        "PAL_UDP_MTU_SIZE=1358",
        "BAK__UBLOX_SARA_R410M=1",

2. \mbed-os\features\cellular\framework\targets\QUECTEL\BC95\mbed_lib.json
        "provide-default": {
            "help": "Provide as default CellularDevice [true/false]",
            "value": true
        }
   \mbed-os\features\cellular\framework\targets\UBLOX\AT\mbed_lib.json
   \mbed-os\features\cellular\framework\targets\QUECTEL\BG96\mbed_lib.json
        "provide-default": {
            "help": "Provide as default CellularDevice [true/false]",
            "value": false
        }

3. \common_def.h
	#define _DEVICE_QUECTEL_BC95
	#define x_DEVICE_UBLOX_SARA_R410M
	#define x_DEVICE_QUECTEL_BG96