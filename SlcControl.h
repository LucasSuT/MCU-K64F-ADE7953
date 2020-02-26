
#ifndef _SLCCONTROL_H
#define _SLCCONTROL_H

#include "SlcControlData.h"

#define SPI_TRANSFER_SIZE       (INDEX_SPIMASTER_LAST_CHECK_BYTE+1) //SPIMASTER_MAX_LEN //(INDEX_SPIMASTER_LAST_CHECK_BYTE+1) //(32 + GPS_DATA_MAX)

void SlcCtrl_Init(void);
float SlcCtrl_GetVoltage(void);
float SlcCtrl_GetFreq(void);
float SlcCtrl_GetPF(void);
float SlcCtrl_GetCurrent(void);
float SlcCtrl_GetWatt(void);
float SlcCtrl_GetTotalWatt(void);
void SlcCtrl_GetDimLevel(uint8_t *buff, int buff_size);
void SlcCtrl_SetDimLevel(uint8_t broadcast, uint8_t *buff, int buff_size);
uint16_t SlcCtrl_GetLux(void);
uint8_t SlcCtrl_GetMcuMode(void);
uint8_t SlcCtrl_GetMcuVersion(void);
#ifdef _FEATURE_GPS_ON_MICROCHIP
void SlcCtrl_GetGpsData(uint8_t *buff, int buff_size);
#endif
uint8_t SlcCtrl_GetSensorMode(void);
void SlcCtrl_SetMcuMode(uint8_t value);

#ifdef _PROJECT_SLC_COSTDOWN
void SlcCtrl_VoltageOut_UnitTest(void);
#endif

#endif // #ifndef _SLCCONTROL_H
