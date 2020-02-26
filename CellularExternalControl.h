#ifndef _CELLULAR_EXTERNAL_CONTROL
#define _CELLULAR_EXTERNAL_CONTROL


int CellularExt_GetModelIdentification(char *name, int size);
int CellularExt_GetImei(char *imei, int size);
int CellularExt_GetImsi(char *imsi, int size);
int CellularExt_GetFwVersion(char *version, int size);
int CellularExt_GetCcid(char *ccid, int size);
int CellularExt_CustomAtCmd(char *cmd, char *rsp, int size);

int CellularExt_GetSignalQuality(int *rssi, int *ber);
int CellularExt_GetSignalStrength(int *rsrp, int *rsrq, int *snr);
int CellularExt_GetCellID(uint32_t *cellid);
int CellularExt_GPSEn(bool en);
int CellularExt_SetNbiot(void);
int CellularExt_SetCatM1(void);
int CellularExt_SetAutoCatM1Nbiot(void);
int CellularExt_GetBand(uint8_t *band, uint32_t size, uint8_t *bandcnt);
int CellularExt_SetBand(uint8_t *band, uint32_t size);
int CellularExt_ScramblingEn(bool en);
int CellularExt_SetAPN(char *apn);
int CellularExt_ClearAPN(void);
int CellularExt_GetPsm(bool *psm);
int CellularExt_GetEdrx(bool *edrx);
int CellularExt_GetEdrxrdp(void);

int CellularExt_SWReset_Reset(void);
void CellularExt_HWReset_Reset(void);
void CellularExt_HWReset_Set(int high);
int CellularExt_PreInitModule(void);
void CellularExt_DisconnectRAI(void);
void CellularExt_Attach(void);
void CellularExt_Detach(void);

void run_some_modem_command();


#endif

