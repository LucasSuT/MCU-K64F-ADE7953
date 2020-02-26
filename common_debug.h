#ifndef _DEBUG_H
#define _DEBUG_H

#include "mbed.h"

/*
example :
    copy below inlcude and define in your source code file
    you can easy to enable/disable the debug message in that source code
    just define or undefine _DEBUG_xxxx in here

#include "common_debug.h"

#ifdef _DEBUG_xxxx
#define TAG         	"xxxx.c"
#define xxxx_DEBUG  	DEBUGMSG
#define xxxx_DUMP		DEBUGMSG_DUMP
#else
#define xxxx_DEBUG
#define xxxx_DUMP
#endif //_DEBUG_xxxx
*/

//------------------------------------------//
// function declaration
//------------------------------------------//
#define _DEBUG_ERROR_ENABLE                 // enable/disable error message
#define _DEBUG_ENABLE                       // enable/disable debug message
#define _DEBUG_TAG_LEVEL    4               // 0: no tag
                                            // 1: only tag
                                            // 2: tag + line number
                                            // 3: tag + function name
                                            // 4: tag + function name + line number

//------------------------------------------//
// module debug declaration
// to enable/disable debug message of each module
//------------------------------------------//
#define _DEBUG_MAIN							// main.cpp
#define _DEBUG_CELLULAREXT					// CellularExternalControl.cpp
#define x_DEBUG_DATASTORE					// DataStore.cpp

// for _PROJECT_IZANAGI
#define x_DEBUG_IZANAGIAPP					// IzanagiApplication.cpp
#define x_DEBUG_IZANAGILWM2MDATA			// IzanagiLwm2mData.cpp
#define x_DEBUG_EXTSENSOR					// ExternalSensors.cpp

// for _PROJECT_SLC
#define _DEBUG_SLCAPP						// SlcApplication.cpp
#define x_DEBUG_SLCCTRL						// SlcControl.cpp
#define x_DEBUG_SLCLWM2MDATA				// SlcLwm2mData.cpp


//------------------------------------------//
// content
//------------------------------------------//
#ifdef _DEBUG_ENABLE

#if _DEBUG_TAG_LEVEL >= 4
    #define DEBUGMSG(str, args...)              do{ printf("[%s:%s:%d]: " str "\r\n", TAG, __FUNCTION__, __LINE__, ##args); } while (0)
    #define DEBUGMSG_NOENTER(str, args...)      do{ printf("[%s:%s:%d]: " str , TAG, __FUNCTION__, __LINE__, ##args); } while (0)
    #define DEBUGMSG_DUMP(str, i, size, buff)       \
                                do{ printf("[%s:%s:%d]: " str "[%d]: ", TAG, __FUNCTION__, __LINE__, size); \
                                    for(i=0; i<size; i++) \
                                        printf("%02x ", buff[i]); \
                                    printf("\r\n");} while (0)
#elif _DEBUG_TAG_LEVEL == 3
    #define DEBUGMSG(str, args...)              do{ printf("[%s:%s]: " str "\r\n", TAG, __FUNCTION__, ##args); } while (0)
    #define DEBUGMSG_NOENTER(str, args...)      do{ printf("[%s:%s]: " str , TAG, __FUNCTION__, ##args); } while (0)
    #define DEBUGMSG_DUMP(str, i, size, buff)       \
                                do{ printf("[%s:%s]: " str "[%d]: ", TAG, __FUNCTION__, size); \
                                    for(i=0; i<size; i++) \
                                        printf("%02x ", buff[i]); \
                                    printf("\r\n");} while (0)
#elif _DEBUG_TAG_LEVEL == 2
    #define DEBUGMSG(str, args...)              do{ printf("[%s:%d]: " str "\r\n", TAG, __LINE__, ##args); } while (0)
    #define DEBUGMSG_NOENTER(str, args...)      do{ printf("[%s:%d]: " str , TAG, __LINE__, ##args); } while (0)
    #define DEBUGMSG_DUMP(str, i, size, buff)       \
                                do{ printf("[%s:%d]: " str "[%d]: ", TAG, __LINE__, size); \
                                    for(i=0; i<size; i++) \
                                        printf("%02x ", buff[i]); \
                                    printf("\r\n");} while (0)
#elif _DEBUG_TAG_LEVEL == 1
    #define DEBUGMSG(str, args...)              do{ printf("[%s]: " str "\r\n", TAG, ##args); } while (0)
    #define DEBUGMSG_NOENTER(str, args...)      do{ printf("[%s]: " str , TAG, ##args); } while (0)
    #define DEBUGMSG_DUMP(str, i, size, buff)       \
                                do{ printf("[%s]: " str "[%d]: ", TAG, size); \
                                    for(i=0; i<size; i++) \
                                        printf("%02x ", buff[i]); \
                                    printf("\r\n");} while (0)
#elif _DEBUG_TAG_LEVEL <= 0
    #define DEBUGMSG(str, args...)              do{ printf( str "\r\n", ##args); } while (0)
    #define DEBUGMSG_NOENTER(str, args...)      do{ printf( str , ##args); } while (0)
    #define DEBUGMSG_DUMP(str, i, size, buff)       \
                                do{ printf( str "[%d]: ", size); \
                                    for(i=0; i<size; i++) \
                                        printf("%02x ", buff[i]); \
                                    printf("\r\n");} while (0)
#endif

#define DEBUGMSG_NOENTER_NOTAG(str, args...)    do{printf(str, ##args);} while (0)

#define DEBUGMSG_S(str, args...)                do{ printf(str"\r\n", ##args); } while (0)
#define DEBUGMSG_NOENTER_S(str, args...)        do{ printf(str, ##args); } while (0)


#define NODEF_DEBUGMSG(str, args...)
#define NODEF_DEBUGMSG_NOENTER(str, args...)
#define NODEF_DEBUGMSG_NOENTER_NOTAG(str, args...)
#define NODEF_DEBUGMSG_S(str, args...)
#define NODEF_DEBUGMSG_NOENTER_S(str, args...)

#else // #ifdef _DEBUG_ENABLE
#define DEBUGMSG(str, args...)
#define DEBUGMSG_NOENTER(str, args...)
#define DEBUGMSG_NOENTER_NOTAG(str, args...)
#define NODEF_DEBUGMSG_S(str, args...)
#define NODEF_DEBUGMSG_NOENTER_S(str, args...)
#define NODEF_DEBUGMSG(str, args...)
#define NODEF_DEBUGMSG_NOENTER(str, args...)
#define NODEF_DEBUGMSG_NOENTER_NOTAG(str, args...)
#define NODEF_DEBUGMSG_S(str, args...)
#define NODEF_DEBUGMSG_NOENTER_S(str, args...)
#endif // #else // #ifdef _DEBUG_ENABLE

#ifdef _DEBUG_ERROR_ENABLE
#define DEBUGMSG_ERR(str, args...)              do{ printf("[ERR:%s:%d]: " str "\r\n", __FILE__, __LINE__, ##args); } while (0)
#define DEBUGMSG_NOENTER_ERR(str, args...)      do{ printf("[ERR:%s:%d]: " str , __FILE__, __LINE__, ##args); } while (0)
#else
#define DEBUGMSG_ERR(str, args...)
#define DEBUGMSG_NOENTER_ERR(str, args...)
#endif

#endif // _DEBUG_H
