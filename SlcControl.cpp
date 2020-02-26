#include "common_def.h"
#if !defined(_PROJECT_SLC) && !defined(_PROJECT_SLC_FTM)


void SlcCtrl_Init(void){;}
float SlcCtrl_GetVoltage(void) {return 0;}
float SlcCtrl_GetFreq(void) {return 0;}
float SlcCtrl_GetPF(void) {return 0;}
float SlcCtrl_GetCurrent(void) {return 0;}
float SlcCtrl_GetWatt(void) {return 0;}
float SlcCtrl_GetTotalWatt(void) {return 0;}
void SlcCtrl_GetDimLevel(uint8_t *buff, int buff_size)
{
	buff[0] = 1;
	buff[1] = 0;
}
void SlcCtrl_SetDimLevel(uint8_t broadcast, uint8_t *buff, int buff_size){;}
uint16_t SlcCtrl_GetLux(void){return 0;}
uint8_t SlcCtrl_GetMcuMode(void){return 0;}
uint8_t SlcCtrl_GetMcuVersion(void){return 0;}
uint8_t SlcCtrl_GetSensorMode(void){return 0;}
void SlcCtrl_SetMcuMode(uint8_t value){;}


#else // #if !defined(_PROJECT_SLC) && !defined(_PROJECT_SLC_FTM)


//#include "mbed_include.h"
#include "mbed.h"
#include "SlcControl.h"
#include "SlcControlData.h"


#include "common_debug.h"
#ifdef _DEBUG_SLCCTRL
#define TAG				"SCLCTRL"
#define SLCCTRL_DEBUG	DEBUGMSG
#define SLCCTRL_DUMP	DEBUGMSG_DUMP
#else
#define SLCCTRL_DEBUG
#define SLCCTRL_DUMP
#endif //_DEBUG_SCLCTRL



#ifdef _PROJECT_SLC_COSTDOWN
#include "SlcPinDefine.h"

//-----------------------------------------------------------------------------------//
// constant definition
//-----------------------------------------------------------------------------------//
#define DIM_MIN						0
#define DIM_MAX						100

#define VOLTAGEOUT_PWM_DELAY_STEP	4


//-----------------------------------------------------------------------------------//
// global variables
//-----------------------------------------------------------------------------------//
typedef struct
{
	bool TimerThreadWatchdog;	// flag for launch timer

	uint8_t CurrentDim;
} _SlcCtrl;

static _SlcCtrl SlcCtrl;


//-----------------------------------------------------------------------------------//
// driver
//-----------------------------------------------------------------------------------//
#define VOLTAGEOUT_RELAY_PIN		AC_Control
#define VOLTAGEOUT_PWM_PIN			DALI_PWM
#define VOLTAGEOUT_PWM_PERIOD_US	624

DigitalOut ACPower(AC_Control);
PwmOut VoltageOut(VOLTAGEOUT_PWM_PIN);

#define ACPOWER_ON()			ACPower = 1
#define ACPOWER_OFF()			ACPower = 0

//VoltageOut.period(4.0f);      // 4 second period
//VoltageOut.write(0.50f);      // 50% duty cycle, relative to period
//VoltageOut = 0.5f;          // shorthand for led.write()
//VoltageOut.pulsewidth(2);   // alternative to led.write, set duty cycle time in seconds

static void VoltageOut_SetPWMGradually(uint8_t current_dim, uint8_t new_dim, uint8_t step)
{
	int diff;
	int gap;
	int dim;
	int true_step;
	int i;
	float duty;
	
	if( (new_dim == current_dim) || (step == 0) )
	{
		duty = (float)dim/100;
		SLCCTRL_DEBUG("duty cycle: %f", duty);
		VoltageOut.write(duty);
	}
	else
	{
		diff = (int)new_dim - (int)current_dim;
		
		if (diff < 20)
			true_step = 1;
		else if (diff <= 35)
			true_step = 2;
		//else if (diff <= 100)
		else
			true_step = (int)step;

		gap = diff / true_step;
		
		for (i = 1; i <= true_step; i++) {
			if(    ((diff > 0) && (dim > new_dim))
				|| ((diff < 0) && (dim < new_dim))
				|| (i == true_step) )
			{
				dim = (int)new_dim;
			}
			else
				dim = (int)current_dim + (i * gap);

			duty = (float)dim/100;
			SLCCTRL_DEBUG("duty cycle: %f", duty);
			VoltageOut.write(duty);
			wait_ms(25);
		}
	}	
}
	
static void VoltageOut_SetPWM(uint8_t dim)
{
	float duty;

	duty = (float)dim/100;
	SLCCTRL_DEBUG("duty cycle: %f", duty);
	VoltageOut.write(duty);
}

static void Light_SetDim(uint8_t dim)
{
	if(dim == 0)
	{
		VoltageOut_SetPWMGradually(SlcCtrl.CurrentDim, dim, VOLTAGEOUT_PWM_DELAY_STEP);
		ACPOWER_OFF();
	}
	else
	{
		if(dim > DIM_MAX)
			dim = DIM_MAX;
		ACPOWER_ON();
	    VoltageOut_SetPWMGradually(SlcCtrl.CurrentDim, dim, VOLTAGEOUT_PWM_DELAY_STEP);
    }
    SlcCtrl.CurrentDim = dim;
	
	SLCCTRL_DEBUG("Light_SetDim: %d", dim);
}

void SlcCtrl_Init(void)
{
	SlcCtrl.CurrentDim = 0;

	VoltageOut.period_us(VOLTAGEOUT_PWM_PERIOD_US);
	
	// turn power on, set dim to max
	Light_SetDim(DIM_MAX);

	SLCCTRL_DEBUG("SlcCtrl_Init complete");
}
float SlcCtrl_GetVoltage(void)
{}
float SlcCtrl_GetFreq(void)
{}
float SlcCtrl_GetPF(void)
{}
float SlcCtrl_GetCurrent(void)
{}
float SlcCtrl_GetWatt(void)
{}
float SlcCtrl_GetTotalWatt(void)
{}
void SlcCtrl_GetDimLevel(uint8_t *buff, int buff_size)
{
	int i;

	#ifdef _FEATURE_DALIADDRESS_GPSONCHIP
	// need to modify for DALI
	//buff_size = (buff_size<LIGHT_ADDR_MAX)? buff_size : LIGHT_ADDR_MAX;
	//SlcCtrl_GetMasterBuff(INDEX_SPIMASTER_DIM_LEVEL, buff, buff_size);
	//SLCCTRL_DUMP("get dim", i, buff_size, buff);
	#else
	memset(buff, SlcCtrl.CurrentDim, buff_size);
	SLCCTRL_DEBUG("get dim: %d", SlcCtrl.CurrentDim);
	SLCCTRL_DUMP("get dim", i, buff_size, buff);
	#endif
}
void SlcCtrl_SetDimLevel(uint8_t broadcast, uint8_t *buff, int buff_size)
{
	#ifdef _FEATURE_DALIADDRESS_GPSONCHIP
	// need to modify for DALI
	//int i;
	//buff_size = (buff_size<LIGHT_ADDR_MAX)? buff_size : LIGHT_ADDR_MAX;
	//SlcCtrl_SetSlaveBuff(IDX_SPISLAVE_DIM, buff, buff_size);
	//SlcCtrl_SetSlaveByte(IDX_SPISLAVE_DIM_BROADCAST, broadcast);
	//if(broadcast)
	//	SLCCTRL_DEBUG("set dim broadcast");
	//SLCCTRL_DUMP("set dim", i, buff_size, buff);
	#else
	Light_SetDim(buff[0]);
	SLCCTRL_DEBUG("set dim: %d", buff[0]);
	#endif
}
uint16_t SlcCtrl_GetLux(void)
{}
uint8_t SlcCtrl_GetMcuMode(void)
{}
uint8_t SlcCtrl_GetMcuVersion(void)
{}
uint8_t SlcCtrl_GetSensorMode(void)
{}
void SlcCtrl_SetMcuMode(uint8_t value)
{}
void SlcCtrl_GetGpsData(uint8_t *buff, int buff_size)
{}


void SlcCtrl_VoltageOut_UnitTest(void)
{
	uint8_t dim;
	wait(10);
	dim = 0;
	while(1)
	{
		//printf("test pwm, set dim: %d\n", dim);
		Light_SetDim(dim);
		wait(10);
		dim += 20;
		if(dim > 100)
			dim = 0;
	}
}

#else // #ifdef _PROJECT_SLC_COSTDOWN

//-----------------------------------------------------------------------------------//
// debug
//-----------------------------------------------------------------------------------//
#define x_SPI_DUMP				// spi driver debug
#ifdef _SPI_DUMP
#define SPI_DUMP_ITEM_MAX	6
#endif

//-----------------------------------------------------------------------------------//
// function declaration
//-----------------------------------------------------------------------------------//
static void SlcCtrl_TimerThreadWatchdogFxn(void const *n);
static void SlcCtrl_KickThreadTimer(void);

//-----------------------------------------------------------------------------------//
// global variables
//-----------------------------------------------------------------------------------//
typedef struct
{
	bool TimerThreadWatchdog;	// flag for launch timer

	char SpiWriteBuffer[SPI_TRANSFER_SIZE];
	char SpiReadBuffer[SPI_TRANSFER_SIZE];
	char SpiReadBackupBuffer[SPI_TRANSFER_SIZE];
	
	#ifdef _SPI_DUMP
	char SpiRxDump[SPI_DUMP_ITEM_MAX][SPI_TRANSFER_SIZE];
	int  SpiRxItem;
	int  SpiRxItemBackup;
	int  SpiRxIdx;
	bool SpiRxDumpFull;
	#endif
} _SlcCtrl;

static _SlcCtrl SlcCtrl;

SPISlave slcctrl_spi(SLC_SPI_MOSI, SLC_SPI_MISO, SLC_SPI_CLK, SLC_SPI_CS);

#ifdef _DEBUG_SLCCTRL
Thread SlcCtrl_Thread(osPriorityNormal, 1024, NULL, "slcctrl_thread"); //(osPriorityNormal, MBED_CONF_APP_SLCCTRL_STACK_SIZE, _slcctrl_stack, "slcctrl_thread");
#else
Thread SlcCtrl_Thread(osPriorityNormal, 512, NULL, "slcctrl_thread");
#endif

RtosTimer SlcCtrl_TimerThreadWatchdog(SlcCtrl_TimerThreadWatchdogFxn, osTimerPeriodic, (void *)0);

//-----------------------------------------------------------------------------------//
// macro declaration
//-----------------------------------------------------------------------------------//
// timer - thread watchdog
#define SLCCTRL_RELAUNCHTIMER_THREADWATCHDOG()	do { \
													if(SlcCtrl.TimerThreadWatchdog) SlcCtrl_TimerThreadWatchdog.stop(); \
													SlcCtrl_TimerThreadWatchdog.start(THREAD_WATCHDOG_TIMEOUTTIME_SEC*1000); \
													SlcCtrl.TimerThreadWatchdog = true; \
												} while(0)


//-----------------------------------------------------------------------------------//
// functiions
//-----------------------------------------------------------------------------------//
static void SlcCtrl_TimerThreadWatchdogFxn(void const *n)
{
	_DateTimeStruct now;
	RTC_GetRealTime(&now);

	SlcCtrl.TimerThreadWatchdog = false;

	SLCCTRL_DEBUG("!!!!!!!! SlcCtrl_TimerThreadWatchdogFxn !!!!!!!!!!!!!!!!!!!");
	SLCCTRL_DEBUG("@ %d-%02d-%02d (w:%d) %02d:%02d:%02d", now.Y, now.M, now.D, now.WD, now.h, now.m, now.s);
	OS_Reboot(DEFAULT_OS_REBOOT_WITH_MODEM_RESET_CONFIG);
}

static void SlcCtrl_KickThreadTimer(void)
{
	//SLCCTRL_DEBUG("reset thread-watchdog timer");
	SLCCTRL_RELAUNCHTIMER_THREADWATCHDOG();
}

//void SlcCtrl_ThreadFxn(void)
static void SlcCtrl_ThreadFxn(void const *args)
{
	char tmp;
	int i, idx;
	bool recData;
	int  spiReadLen;
	
	SLCCTRL_DEBUG("SlcCtrl ThreadFxn");

	SlcCtrl.SpiWriteBuffer[IDX_SPISLAVE_FIRSTCHECK]		= SPI_FIRST_CHECK_BYTE;
	#ifdef _FEATURE_DALIADDRESS_GPSONCHIP
	SlcCtrl.SpiWriteBuffer[IDX_SPISLAVE_DIM_BROADCAST]	= 1;
	#endif
	SlcCtrl.SpiWriteBuffer[IDX_SPISLAVE_LIGHTPAIR]		= SPI_LIGHT_SENSOR_MODE;
	SlcCtrl.SpiWriteBuffer[IDX_SPISLAVE_RESERVE1]		= 0;
	SlcCtrl.SpiWriteBuffer[IDX_SPISLAVE_RESERVE2]		= 0;
	#ifdef _FEATURE_GPS_ON_MICROCHIP
	SlcCtrl.SpiWriteBuffer[IDX_SPISLAVE_GPSDATAFORMAT]	= GPS_SPECIFY_DEFAULT;
	SlcCtrl.SpiWriteBuffer[IDX_SPISLAVE_GPSPOWEREN]		= GPS_POWER_DEFAULT;
	#endif
	memset(&SlcCtrl.SpiWriteBuffer[IDX_SPISLAVE_DIM], 100, LIGHT_ADDR_MAX);
	SlcCtrl.SpiWriteBuffer[IDX_SPISLAVE_LASTCHECK]		= SPI_LAST_CHECK_BYTE;

	/*
	SLCCTRL_DEBUG("spi send data [%d]: ", sizeof(SlcCtrl.SpiWriteBuffer));
	for(i=0; i<sizeof(SlcCtrl.SpiWriteBuffer); i++)
		printf("%02x ", SlcCtrl.SpiWriteBuffer[i]);
	printf("\r\n");
	*/

	#ifdef _SPI_DUMP
	SlcCtrl.SpiRxDumpFull = false;
	SlcCtrl.SpiRxItemBackup = 0;
	SlcCtrl.SpiRxItem = 0;
	SlcCtrl.SpiRxIdx = 0;
	#endif
	
	idx = 0;
	spiReadLen = 0;
	memset(SlcCtrl.SpiReadBackupBuffer, 0, sizeof(SlcCtrl.SpiReadBackupBuffer));
	memset(SlcCtrl.SpiReadBuffer, 0, sizeof(SlcCtrl.SpiReadBuffer));
	//slcctrl_spi.reply(0x00);       //You need to prepare the ACK answer and put it on the bus
	recData = false;
	while(1)
	{
		if(slcctrl_spi.receive())									// check if receive from master
		{
//__PASS_DATA:
			tmp = slcctrl_spi.read();								// Read byte from rx buffer
			if(tmp == SPI_FIRST_CHECK_BYTE)
			{
				//wait_us(1);
				slcctrl_spi.reply(SlcCtrl.SpiWriteBuffer[idx]); 	// Make this the next reply
				SlcCtrl.SpiReadBuffer[idx] = tmp;
				idx++;
				#ifdef _SPI_DUMP
				if(!SlcCtrl.SpiRxDumpFull)
				{
					SlcCtrl.SpiRxDump[SlcCtrl.SpiRxItem][SlcCtrl.SpiRxIdx++] = tmp;
					if(SlcCtrl.SpiRxIdx >= SPI_TRANSFER_SIZE)
					{
						SlcCtrl.SpiRxIdx = 0;
						SlcCtrl.SpiRxItem++;
						if(SlcCtrl.SpiRxItem >= SPI_DUMP_ITEM_MAX)
							SlcCtrl.SpiRxDumpFull = true;
					}
				}
				#endif
				
				while(1)
				{
					//wait_us(1);
					tmp = slcctrl_spi.read();						// Read byte from rx buffer
					//wait_us(1);
					slcctrl_spi.reply(SlcCtrl.SpiWriteBuffer[idx]);	// Make this the next reply
					SlcCtrl.SpiReadBuffer[idx] = tmp;
					idx++;

					#ifdef _SPI_DUMP
					if(!SlcCtrl.SpiRxDumpFull)
					{
						SlcCtrl.SpiRxDump[SlcCtrl.SpiRxItem][SlcCtrl.SpiRxIdx++] = tmp;
						if(SlcCtrl.SpiRxIdx >= SPI_TRANSFER_SIZE)
						{
							SlcCtrl.SpiRxIdx = 0;
							SlcCtrl.SpiRxItem++;
							if(SlcCtrl.SpiRxItem >= SPI_DUMP_ITEM_MAX)
								SlcCtrl.SpiRxDumpFull = true;
						}
					}
					#endif
			
					if(idx >= sizeof(SlcCtrl.SpiReadBuffer))
					{
						if(tmp == SPI_LAST_CHECK_BYTE)
						{
							spiReadLen = idx;
							memcpy(SlcCtrl.SpiReadBackupBuffer, SlcCtrl.SpiReadBuffer, sizeof(SlcCtrl.SpiReadBackupBuffer));
							recData = true;
						}
						else
						{
							//SLCCTRL_DUMP("Error", i, idx, SlcCtrl.SpiReadBuffer);
							/*
							SLCCTRL_DEBUG("no last check byte [%d]: ", idx);
							for(i=0; i<idx; i++)
								printf("%02x ", SlcCtrl.SpiReadBuffer[i]);
							printf("\r\n");
							*/
						}
						idx = 0;
						memset(SlcCtrl.SpiReadBuffer, 0, sizeof(SlcCtrl.SpiReadBuffer));
						break;
					}
				}
			}
			else
			{
				//wait_us(1);
				slcctrl_spi.reply(0x00);
				//SLCCTRL_DEBUG("receive: %02x\r\n", tmp);
				//printf(">%02x\r\n", tmp);

				#ifdef _SPI_DUMP
				if(!SlcCtrl.SpiRxDumpFull)
				{
					SlcCtrl.SpiRxDump[SlcCtrl.SpiRxItem][SlcCtrl.SpiRxIdx++] = tmp;
					if(SlcCtrl.SpiRxIdx >= SPI_TRANSFER_SIZE)
					{
						SlcCtrl.SpiRxIdx = 0;
						SlcCtrl.SpiRxItem++;
						if(SlcCtrl.SpiRxItem >= SPI_DUMP_ITEM_MAX)
							SlcCtrl.SpiRxDumpFull = true;
					}
				}
				#endif

				//goto __PASS_DATA;
			}
		}

		if(recData)
		{
			//SLCCTRL_DUMP("out data", i, SPISLAVE_MAX_LEN, SlcCtrl.SpiWriteBuffer);
			SLCCTRL_DUMP("incomming data", i, spiReadLen, SlcCtrl.SpiReadBackupBuffer);
			recData = false;
		}

		#ifdef _SPI_DUMP
		if(SlcCtrl.SpiRxItemBackup != SlcCtrl.SpiRxItem)
		{
			printf(">>> %d\r\n", SlcCtrl.SpiRxItemBackup);
			SLCCTRL_DUMP("", i, SPI_TRANSFER_SIZE, SlcCtrl.SpiRxDump[SlcCtrl.SpiRxItemBackup]);
			SlcCtrl.SpiRxItemBackup = SlcCtrl.SpiRxItem;
		}
		if(SlcCtrl.SpiRxDumpFull)
		{
			SlcCtrl.SpiRxItem = 0;
			SlcCtrl.SpiRxIdx = 0;
			SlcCtrl.SpiRxDumpFull = false;
		}
		#endif

		SlcCtrl_KickThreadTimer();
	}
}

void SlcCtrl_Init(void)
{
	SLCCTRL_DEBUG("SlcCtrl Init");
	slcctrl_spi.format(8, 0);
	slcctrl_spi.frequency(2000000);

	SlcCtrl.TimerThreadWatchdog = false;
	
		
	SLCCTRL_DEBUG("SlcCtrl Launch");
	//SlcCtrl_Thread.start(SlcCtrl_ThreadFxn);
	SlcCtrl_Thread.start(callback(SlcCtrl_ThreadFxn, (void *)"THSlcCtrl"));
}

//-----------------------------------------------------------------------------------//
// data interface of spi master package (received from microchip)
//-----------------------------------------------------------------------------------//
static void SlcCtrl_GetMasterByte(int idx, uint8_t *value)
{
	*value = SlcCtrl.SpiReadBackupBuffer[idx];
}

static void SlcCtrl_GetMasterWord(int idx, uint16_t *value)
{
	*value = 0;
	*value = SlcCtrl.SpiReadBackupBuffer[idx+1]<<8 | SlcCtrl.SpiReadBackupBuffer[idx];
}

static void SlcCtrl_GetMasterInt(int idx, uint32_t *value)
{
	int i;
	*value = 0;
	for(i=3; i>=0; i--)
	{
		*value <<= 8;
		*value |= SlcCtrl.SpiReadBackupBuffer[idx+i];
	}
}

static void SlcCtrl_GetMasterLong(int idx, uint64_t *value)
{
	int i;
	*value = 0;
	for(i=7; i>=0; i--)
	{
		*value <<= 8;
		*value |= SlcCtrl.SpiReadBackupBuffer[idx+i];
	}
}

static void SlcCtrl_GetMasterBuff(int idx, uint8_t *buff, int buff_size)
{
	memcpy(buff, &SlcCtrl.SpiReadBackupBuffer[idx], buff_size);
}

float SlcCtrl_GetVoltage(void)
{
	uint16_t	value_tmp;
	float		value;
	SlcCtrl_GetMasterWord(INDEX_SPIMASTER_VOLTAGE, &value_tmp);
	value = (float)value_tmp*0.1f;
	SLCCTRL_DEBUG("get voltage: %d -> %f", value_tmp, value);
	return value;
}

float SlcCtrl_GetFreq(void)
{
	uint16_t 	value_tmp;
	float		value;
	SlcCtrl_GetMasterWord(INDEX_SPIMASTER_FREQ, &value_tmp);
	value = (float)value_tmp*0.001f;
	SLCCTRL_DEBUG("get freq: %d -> %f", value_tmp, value);
	return value;
}

float SlcCtrl_GetPF(void)
{
	uint16_t	value_tmp;
	float		value;
	SlcCtrl_GetMasterWord(INDEX_SPIMASTER_PWR_FACTOR, &value_tmp);
	value = (float)value_tmp;
	while(value > 1)
	{
		value /= 10;
	}
	SLCCTRL_DEBUG("get pf: %d -> %f", value_tmp, value);
	return value;
}

float SlcCtrl_GetCurrent(void)
{
	uint32_t 	value_tmp;
	float		value;
	SlcCtrl_GetMasterInt(INDEX_SPIMASTER_CURRENT, &value_tmp);
	value = (float)value_tmp*0.0001f;
	SLCCTRL_DEBUG("get current: %d -> %f", value_tmp, value);
	return value;
}

float SlcCtrl_GetWatt(void)
{
	uint32_t 	value_tmp;
	float		value;
	SlcCtrl_GetMasterInt(INDEX_SPIMASTER_WATT, &value_tmp);
	value = (float)value_tmp*0.01f;
	SLCCTRL_DEBUG("get watt: %d -> %f", value_tmp, value);
	return value;
}

float SlcCtrl_GetTotalWatt(void)
{
	uint64_t 	value_tmp;
	float		value;
	SlcCtrl_GetMasterLong(INDEX_SPIMASTER_TOTAL_WATT, &value_tmp);
	value = (float)value_tmp*0.0000001f;
	SLCCTRL_DEBUG("get total-watt: %lld -> %f", value_tmp, value);
	return value;
}

void SlcCtrl_GetDimLevel(uint8_t *buff, int buff_size)
{
	int i;

	#ifdef _FEATURE_DALIADDRESS_GPSONCHIP
	buff_size = (buff_size<LIGHT_ADDR_MAX)? buff_size : LIGHT_ADDR_MAX;
	SlcCtrl_GetMasterBuff(INDEX_SPIMASTER_DIM_LEVEL, buff, buff_size);
	SLCCTRL_DUMP("get dim", i, buff_size, buff);
	#else
	uint8_t value;
	SlcCtrl_GetMasterByte(INDEX_SPIMASTER_DIM_LEVEL, &value);
	memset(buff, value, buff_size);
	SLCCTRL_DEBUG("get dim: %d", value);
	SLCCTRL_DUMP("get dim", i, buff_size, buff);
	#endif
}

uint16_t SlcCtrl_GetLux(void)
{
	uint8_t lo, hi;
	uint16_t value;
	SlcCtrl_GetMasterByte(INDEX_SPIMASTER_LUX_LO, &lo);
	SlcCtrl_GetMasterByte(INDEX_SPIMASTER_LUX_HI, &hi);

	value = 0;
	value = hi<<8 | lo;
	
	SLCCTRL_DEBUG("get lux: %d", value);
	return value;
}

uint8_t SlcCtrl_GetMcuMode(void)
{
	uint8_t value;
	SlcCtrl_GetMasterByte(INDEX_SPIMASTER_SYSTEM, &value);
	SLCCTRL_DEBUG("get mcu mode: %d", value);
	return value;
}

uint8_t SlcCtrl_GetMcuVersion(void)
{
	uint8_t value;
	SlcCtrl_GetMasterByte(INDEX_SPIMASTER_MCU_VER, &value);
	SLCCTRL_DEBUG("get mcu version: %d", value);
	return value;
}

#ifdef _FEATURE_GPS_ON_MICROCHIP
void SlcCtrl_GetGpsData(uint8_t *buff, int buff_size)
{
	int i;
	buff_size = (buff_size<INDEX_SPIMASTER_GPS_DATA_LEN)? buff_size : INDEX_SPIMASTER_GPS_DATA_LEN;
	SlcCtrl_GetMasterBuff(INDEX_SPIMASTER_GPS_DATA, buff, buff_size);
	SLCCTRL_DUMP("get gps data", i, buff_size, buff);
}
#endif

//-----------------------------------------------------------------------------------//
// data interface of spi slave package (transmiting to microchip)
//-----------------------------------------------------------------------------------//
static void SlcCtrl_SetSlaveByte(int idx, uint8_t value)
{
	SlcCtrl.SpiWriteBuffer[idx] = value;
}

static void SlcCtrl_SetSlaveBuff(int idx, uint8_t *buff, int buff_size)
{
	memcpy(&SlcCtrl.SpiWriteBuffer[idx], buff, buff_size);
}

void SlcCtrl_SetMcuMode(uint8_t value)
{
	SlcCtrl_SetSlaveByte(IDX_SPISLAVE_LIGHTPAIR, value);
	SLCCTRL_DEBUG("set mcu mode: %d", value);
}

void SlcCtrl_SetDimLevel(uint8_t broadcast, uint8_t *buff, int buff_size)
{
	#ifdef _FEATURE_DALIADDRESS_GPSONCHIP
	int i;
	buff_size = (buff_size<LIGHT_ADDR_MAX)? buff_size : LIGHT_ADDR_MAX;
	SlcCtrl_SetSlaveBuff(IDX_SPISLAVE_DIM, buff, buff_size);
	SlcCtrl_SetSlaveByte(IDX_SPISLAVE_DIM_BROADCAST, broadcast);
	if(broadcast)
		SLCCTRL_DEBUG("set dim broadcast");
	SLCCTRL_DUMP("set dim", i, buff_size, buff);
	#else
	SlcCtrl_SetSlaveByte(IDX_SPISLAVE_DIM, buff[0]);
	SLCCTRL_DEBUG("set dim: %d", buff[0]);
	#endif
}

#endif // #else // #ifdef _PROJECT_SLC_COSTDOWN

#endif // #ifdef _PROJECT_SLC

