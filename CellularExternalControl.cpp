#include "mbed.h"
#include "CellularDevice.h"
//#include "AT_CellularNetwork.h"
#include "mcc_common_setup.h"
#include "CellularExternalControl.h"
#include "common_def.h"

#include "common_debug.h"
#ifdef _DEBUG_CELLULAREXT
#define TAG				"CELLULAREXT"
#define CELLULAREXT_DEBUG	DEBUGMSG
#define CELLULAREXT_DUMP	DEBUGMSG_DUMP
#else
#define CELLULAREXT_DEBUG
#define CELLULAREXT_DUMP
#endif //_DEBUG_CELLULAREXT
#define CELLULAREXT_ERR		DEBUGMSG_ERR


const char NAME_BG96[]	= "BG96";
const char NAME_BC95G[]	= "BC95G";
const char NAME_R410M[]	= "R410M-02B";
const char NAME_N410[]	= "N410-02B";


enum {
	CELLMODULE_BG96,
	CELLMODULE_BC95G,
	CELLMODULE_R410M,
	CELLMODULE_N410,
	CELLMODULE_MAX
};

const char *ModuleNameList[CELLMODULE_MAX] = {	NAME_BG96,
												NAME_BC95G,
												NAME_R410M,
												NAME_N410 };

typedef struct _cellular_ext {
	uint8_t module;
} _CellularExtStruct;

_CellularExtStruct CellularExt = {.module = CELLMODULE_MAX};



static nsapi_error_t get_at_info(const char *cmd, char *buf, size_t buf_size, ATHandler *atHandler)
{
    int len = 0;
    CELLULAREXT_DEBUG("sending (%s) to modem...", cmd);
    atHandler->lock();
    atHandler->cmd_start(cmd);
    atHandler->cmd_stop();
    atHandler->set_delimiter(0);
    atHandler->resp_start();
    len = atHandler->read_string(buf, buf_size-1);
    atHandler->resp_stop();
    atHandler->set_default_delimiter();
    return atHandler->unlock_return_error();
}


int CellularExt_GetModelIdentification(char *name, int size)
{
	int ret = -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("GetModelIdentification: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		at->flush(); // flush existing data
		//at->set_debug(1);
		
        ret = (int)get_at_info("AT+CGMM", name, size, at);
        CELLULAREXT_DEBUG("GetModelIdentification: name = %s", name); 
		//ThisThread::sleep_for(1000);
		
		device->release_at_handler(at);
    }
	else
		CELLULAREXT_ERR("GetModelIdentification: get at handler fail");

	wait(0.3);
	return ret;
}

int CellularExt_GetImei(char *imei, int size)
{
	int ret = -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("GetImei: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		at->flush(); // flush existing data
		//at->set_debug(1);

        ret = (int)get_at_info("AT+GSN", imei, size, at);
        CELLULAREXT_DEBUG("GetImei: imei = %s", imei); 
		//ThisThread::sleep_for(1000);

		device->release_at_handler(at);
    }
	else
		CELLULAREXT_ERR("GetImei: get at handler fail");

	wait(0.3);
	return ret;
}

int CellularExt_GetImsi(char *imsi, int size)
{
	int ret = -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("GetImsi: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		at->flush(); // flush existing data
		//at->set_debug(1);

        ret = (int)get_at_info("AT+CIMI", imsi, size, at);
        CELLULAREXT_DEBUG("GetImsi: imsi = %s", imsi); 
		//ThisThread::sleep_for(1000);

		device->release_at_handler(at);
    }
	else
		CELLULAREXT_ERR("GetImsi: get at handler fail");

	wait(0.3);
	return ret;
}

int CellularExt_GetFwVersion(char *version, int size)
{
	int ret = -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("GetFwVersion: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		at->flush(); // flush existing data
		//at->set_debug(1);

        ret = (int)get_at_info("AT+CGMR", version, size, at);
        CELLULAREXT_DEBUG("GetFwVersion: fw version = %s", version); 
		//ThisThread::sleep_for(1000);

		device->release_at_handler(at);
    }
	else
		CELLULAREXT_ERR("GetFwVersion: get at handler fail");

	wait(0.3);
	return ret;
}

int CellularExt_GetCcid(char *ccid, int size)
{
	int ret = -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("GetCcid: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		
		at->flush(); // flush existing data

        //ret = (int)get_at_info("AT+CCID", ccid, size, at);
	    at->lock();
		at->cmd_start("AT+CCID");
		at->cmd_stop();
		//at->set_delimiter(0);
		at->resp_start("+CCID: ");
		at->read_string(ccid, size);
		at->resp_stop();
		//at->set_default_delimiter();
		ret = at->unlock_return_error();
		CELLULAREXT_DEBUG("GetCcid: ccid = %s", ccid);

		device->release_at_handler(at);
    }
	else
		CELLULAREXT_ERR("GetCcid: get at handler fail");

	wait(0.3);
	return ret;
}

int CellularExt_CustomAtCmd(char *cmd, char *rsp, int size)
{
	int ret = -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("CustomAtCmd: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		at->flush(); // flush existing data
		//at->set_debug(1);

        ret = (int)get_at_info(cmd, rsp, size, at);
        CELLULAREXT_DEBUG("CustomAtCmd: rsp = %s", rsp); 
		//ThisThread::sleep_for(1000);

		device->release_at_handler(at);
    }
	else
		CELLULAREXT_ERR("CustomAtCmd: get at handler fail");

	wait(0.3);
	return ret;
}

int CellularExt_GetSignalQuality(int *rssi, int *ber)
{
	int ret = -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("GetSignalQuality: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+CSQ");
			
		at->flush(); // flush existing data
		//at->set_debug(1);
		at->lock();
		at->cmd_start("AT+CSQ");
		at->cmd_stop();
		at->set_delimiter(0);
		at->resp_start("+CSQ:");
		*rssi = at->read_int();
		*ber = at->read_int();
		at->resp_stop();
		at->set_default_delimiter();
		CELLULAREXT_DEBUG("CellularExt_GetSignalQuality 1: rssi: %d, ber: %d", *rssi, *ber);
		if (*rssi < 0) {
			at->unlock();
			ret = NSAPI_ERROR_DEVICE_ERROR;
		}
		else
		{
			if( (*rssi>=0) && (*rssi<=31) ) {
				*rssi = -113 + 2 * (*rssi);
			}
			else {
				*rssi = 0;
			}
			ret = at->unlock_return_error();
			CELLULAREXT_DEBUG("CellularExt_GetSignalQuality 2: rssi: %d, ber: %d", *rssi, *ber);
		}
		device->release_at_handler(at);
    }
	else
	{
		CELLULAREXT_ERR("GetSignalQuality: get at handler fail");
		ret = -1;
	}

	wait(0.3);
	return ret;
}

static bool CellularExt_EnSignalReport = false;
static int CellularExt_EnableSignalStrengthReport(void)
{
#ifdef _DEVICE_UBLOX_SARA_R410M
	int ret = -1;
	int retry = 0;

	if(CellularExt_EnSignalReport)
		return true;
	
	// take the ATHandler into use for sending ad hoc commands to the modem
	CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("CellularExt_GetSignalStrength: get cellular device fail");
		return ret;
	}
	ATHandler *at = device->get_at_handler();

	if (at)
	{
		CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+UCGED=5");
		at->flush(); // flush existing data
		//at->set_debug(1);
__CMD_RESEND:
		at->lock();
		at->cmd_start("AT+UCGED=5");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		ret = at->unlock_return_error();
		if(ret < 0)
		{
			retry++;
			if(retry < 3)
			{
				wait(2);
				goto __CMD_RESEND;
			}
		}
		ret = device->release_at_handler(at);
	}
	else
	{
		CELLULAREXT_ERR("CellularExt_GetSignalStrength: get at handler fail");
	}

	wait(0.3);
	if(!ret)
		CellularExt_EnSignalReport = true;
	return ret;
#else
	return 0;
#endif
}

int CellularExt_GetSignalStrength(int *rsrp, int *rsrq, int *snr)
{
	int i, idx_start, idx_end, passed;
	int ret = -1;
	int rssi, earfcn, phycellid, primarycell;
	float rsrpf, rsrqf;
	char strrsrp[200];
	char strrsrq[200];
	char strtmp[10];
	int rsrplen, rsrqlen;

	rssi = 0;
	earfcn = 0;
	phycellid = 0;
	primarycell = 0;\
	rsrpf = 0;
	rsrqf = 0;
	*rsrp = 0;
	*rsrq = 0;
	*snr = 0;

	CellularExt_EnableSignalStrengthReport();
	wait(2);

	memset(strtmp, 0, sizeof(strtmp));
	memset(strrsrp, 0, sizeof(strrsrp));
	memset(strrsrq, 0, sizeof(strrsrq));

	// take the ATHandler into use for sending ad hoc commands to the modem
	CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("CellularExt_GetSignalStrength: get cellular device fail");
		return ret;
	}
	ATHandler *at = device->get_at_handler();

	if (at)
	{
		switch(CellularExt.module)
		{
			case CELLMODULE_BG96:
				CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+QCSQ");
				at->flush(); // flush existing data
				//at->set_debug(1);
				at->lock();
				at->cmd_start("AT+QCSQ");
				at->cmd_stop();
				at->set_delimiter(0);
				at->resp_start("+QCSQ:");
				rssi		= at->read_int();				// rssi
				*rsrp		= at->read_int();				// rsrp
				*snr		= at->read_int();				// sinr
				*rsrq		= at->read_int();				// rsrp
				at->resp_stop();
				at->set_default_delimiter();
				ret = at->unlock_return_error();
				CELLULAREXT_DEBUG("CellularExt_GetSignalStrength rssi:%d, rsrp:%d, sinr:%d, rsrq", rssi, *rsrp, *snr, *rsrq);
				break;
			case CELLMODULE_BC95G:
				CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+NUESTATS=CELL");
				at->flush(); // flush existing data
				//at->set_debug(1);
				at->lock();
				at->cmd_start("AT+NUESTATS=CELL");
				at->cmd_stop();
				at->resp_start("NUESTATS:CELL,");
				earfcn		= at->read_int();		// earfcn
				phycellid	= at->read_int();		// physical cell id
				primarycell	= at->read_int();		// primarycell
				*rsrp		= at->read_int();		// rsrp
				*rsrq		= at->read_int();		// rsrq
				rssi		= at->read_int();		// rssi
				*snr		= at->read_int();		// snr
				at->resp_stop();
				//at->set_debug(0);
				ret = at->unlock_return_error();
				CELLULAREXT_DEBUG("CellularExt_GetSignalStrength earfcn:%d, phycellid:%d, primarycell: %d, rssi:%d, rsrp:%d, rsrq:%d, snr:%d", 
															earfcn, phycellid, primarycell, rssi, *rsrp, *rsrq, *snr);
				break;
			case CELLMODULE_R410M:
			case CELLMODULE_N410:
				CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+UCGED?");
				at->flush(); // flush existing data
				//at->set_debug(1);
				at->lock();
				// to query rsrp
				at->cmd_start("AT+UCGED?");
				at->cmd_stop();
				at->set_delimiter(0);
				at->resp_start("+RSRP:");
				// it parse failed by at api, so parse by myself
				//phycellid	= at->read_int();		// P-CID
				//earfcn		= at->read_int();		// earfcn
				//rsrpf		= at->read_float();		// rsrp
				rsrplen = at->read_string(strrsrp, sizeof(strrsrp));		// rsrp
				at->resp_stop();
				// to query rsrq
				at->cmd_start("AT+UCGED?");
				at->cmd_stop();
				at->set_delimiter(0);
				at->resp_start("+RSRQ:");
				// it parse failed by at api, so parse by myself
				//phycellid	= at->read_int();		// P-CID
				//earfcn		= at->read_int();		// earfcn
				//rsrqf		= at->read_float();		// rsrq
				rsrqlen = at->read_string(strrsrq, sizeof(strrsrq));		// rsrq
				at->resp_stop();
				at->set_default_delimiter();
				ret = at->unlock_return_error();
				//CELLULAREXT_DEBUG("strrsrp[%d]: %s", rsrplen, strrsrp);
				//CELLULAREXT_DUMP("strrsrp", i, rsrplen, strrsrp);
				//CELLULAREXT_DEBUG("strrsrq[%d]: %s", rsrqlen, strrsrq);
				//CELLULAREXT_DUMP("strrsrq", i, rsrqlen, strrsrq);
				if(rsrplen > 0)
				{
					passed = false;
					idx_start = rsrplen;
					idx_end = rsrplen;
					for(i=0; i<rsrplen; i++)
					{
						if(strrsrp[i] == ',')
						{
							if(!passed)
								passed = true;
							else
							{
								if(idx_start == rsrplen)
								{
									idx_start = i;
									//CELLULAREXT_DEBUG("find 1st mark: %d", idx_start);
								}
								else
								{
									idx_end = i;
									//CELLULAREXT_DEBUG("find 2nd mark: %d", idx_end);
									break;
								}
							}
						}
					}
					if(idx_start < rsrplen)
					{
						memcpy(strtmp, &strrsrp[idx_start+1], (idx_end-idx_start));
						//CELLULAREXT_DEBUG("strtmp: %s\n", strtmp);
						//CELLULAREXT_DUMP("strtmp", i, (idx_end-idx_start), strtmp);
						rsrpf = strtof(strtmp, NULL);
					}
				}
				if(rsrqlen > 0)
				{
					passed = false;
					idx_start = rsrqlen;
					idx_end = rsrqlen;
					for(i=0; i<rsrqlen; i++)
					{
						if(strrsrq[i] == ',')
						{
							if(!passed)
								passed = true;
							else
							{
								if(idx_start == rsrqlen)
								{
									idx_start = i;
									//CELLULAREXT_DEBUG("find 1st mark: %d", idx_start);
								}
								else
								{
									idx_end = i;
									//CELLULAREXT_DEBUG("find 2nd mark: %d", idx_end);
									break;
								}
							}
						}
					}
					if(idx_start < rsrplen)
					{
						memcpy(strtmp, &strrsrq[idx_start+1], (idx_end-idx_start));
						//CELLULAREXT_DEBUG("strtmp: %s", strtmp);
						//CELLULAREXT_DUMP("strtmp", i, (idx_end-idx_start), strtmp);
						rsrqf = strtof(strtmp, NULL);
					}
				}
				*rsrp = (int)rsrpf;
				*rsrq = (int)rsrqf;
				CELLULAREXT_DEBUG("CellularExt_GetSignalStrength P-CID:%d, earfcn:%d, rsrp:%d(%f), rsrq:%d(%f)", 
															phycellid, earfcn, *rsrp, rsrpf, *rsrq, rsrqf);
				break;
		}
		device->release_at_handler(at);
	}
	else
	{
		CELLULAREXT_ERR("CellularExt_GetSignalStrength: get at handler fail");
	}

	wait(0.3);
	return ret;
}


int CellularExt_GetCellID(uint32_t *cellid)
{

	int ret = -1;
	int rssi, earfcn, primarycell;
	char buff_tac[10];
	char buff_ci[10];
	int len = 0;
		
	*cellid = 0;

	CellularExt_EnableSignalStrengthReport();
	wait(2);

	// take the ATHandler into use for sending ad hoc commands to the modem
	CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("CellularExt_GetCellID: get cellular device fail");
		return ret;
	}
	ATHandler *at = device->get_at_handler();

	if (at)
	{
		switch(CellularExt.module)
		{
			case CELLMODULE_BG96:
				memset(buff_tac, 0, sizeof(buff_tac));
				memset(buff_ci, 0, sizeof(buff_ci));
				at->flush(); // flush existing data
				//at->set_debug(1);			
				CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+CEREG=2");
				at->lock();
				at->cmd_start("AT+CEREG=2");
				at->cmd_stop();
				at->set_delimiter(0);
				at->resp_start("+CEREG:");
				at->read_int();											// n
				at->read_int();											// stat
				at->read_hex_string(buff_tac, sizeof(buff_tac)-1);		// tac
				len = at->read_hex_string(buff_ci, sizeof(buff_ci)-1);	// ci
				at->read_int();											// act
				at->resp_stop();
				at->set_default_delimiter();
				ret = at->unlock_return_error();
				if(len > 0)
				{
					*cellid = 0;
					for(int i=0; i<len; i++)
					{
						*cellid <<= 8;
						*cellid |= buff_ci[i];
					}
					CELLULAREXT_DEBUG("CellularExt_GetCellID: 0x%x(%d)", *cellid, *cellid);
				}
		        else
					CELLULAREXT_DEBUG("CellularExt_GetCellID: fail");
				break;
			case CELLMODULE_BC95G:				
				CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+NUESTATS=CELL");
				at->flush(); // flush existing data
				//at->set_debug(1);
				at->lock();
				at->cmd_start("AT+NUESTATS=CELL");
				at->cmd_stop();
				at->resp_start("NUESTATS:CELL,");
				earfcn		= at->read_int();		// earfcn
				*cellid		= at->read_int();		// physical cell id
				primarycell	= at->read_int();		// primarycell
				at->resp_stop();
				ret = at->unlock_return_error();
				CELLULAREXT_DEBUG("CellularExt_GetCellID phycellid:%d", *cellid);
				break;
			case CELLMODULE_R410M:
			case CELLMODULE_N410:
				CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+UCGED?");
				at->flush(); // flush existing data
				//at->set_debug(1);
				at->lock();
				at->cmd_start("AT+UCGED?");
				at->cmd_stop();
				at->set_delimiter(0);
				at->resp_start("+RSRP: ");
				*cellid = at->read_int();		// P-CID
				at->resp_stop();
				at->set_default_delimiter();
				ret = at->unlock_return_error();
				CELLULAREXT_DEBUG("CellularExt_GetCellID P-CID:%d", *cellid);
				break;
		}
		device->release_at_handler(at);
	}
	else
	{
		CELLULAREXT_ERR("CellularExt_GetCellID: get at handler fail");
	}
	
	wait(0.3);
	return ret;
}


int CellularExt_GPSEn(bool en)
{
	int ret = -1;

	CELLULAREXT_DEBUG("CellularExt_GPSEn %d", en);

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("GPSEn: get cellular device fail");
		return -1;
	}
	
    ATHandler *at = device->get_at_handler();

    if (at) {
		if(en)
			CELLULAREXT_DEBUG("sending (%s) to modem...", " AT+QGPS=1");
		else
			CELLULAREXT_DEBUG("sending (%s & %s) to modem...", " AT+QGPS=0", "AT+QGPSEND");

		at->flush(); // flush existing data
		//at->set_debug(1);
		at->lock();
		if(en)
		{
			at->cmd_start("AT+QGPS=1");
			at->cmd_stop();
			at->resp_start();
			at->resp_stop();
		}
		else
		{
			at->cmd_start("AT+QGPS=0");
			at->cmd_stop();
			at->resp_start();
			at->resp_stop();
			
			at->cmd_start("AT+QGPSEND");
			at->cmd_stop();
			at->resp_start();
			at->resp_stop();
		}
		ret = at->unlock_return_error();
		device->release_at_handler(at);
    }
	else
	{
		CELLULAREXT_ERR("GPSEn: get at handler fail");
		ret = -1;
	}

	wait(0.3);
	return ret;
}

int CellularExt_SetNbiot(void)
{	
	int ret = -1;

	printf("CellularExt_SetNbiot\n");

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("SetNbiot: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {

		CELLULAREXT_DEBUG("sending (%s & %s & %s & %s) to modem...", "AT+QCFG=\"nwscanseq\",03", "AT+QCFG=\"nwscanmode\",0,0", "AT+QCFG=\"iotopmode\",1,1", "AT+CFUN=1,1");
	
		at->flush(); // flush existing data
		//at->set_debug(1);
		at->lock();
		at->cmd_start("AT+QCFG=\"nwscanseq\",03,1");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		at->cmd_start("AT+QCFG=\"nwscanmode\",0,1");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		at->cmd_start("AT+QCFG=\"iotopmode\",1,1");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		at->cmd_start("AT+CFUN=1,1");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		ret = at->unlock_return_error();
		device->release_at_handler(at);
    }
	else
	{
		CELLULAREXT_ERR("SetNbiot: get at handler fail");
		ret = -1;
	}

	printf("CellularExt_SetNbiot: wait for 10 secs\n");
	wait(10);
	return ret;
}

int CellularExt_SetCatM1(void)
{	
	int ret = -1;

	printf("CellularExt_SetCatM1\n");
	
    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("SetCatM1: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {

		CELLULAREXT_DEBUG("sending (%s & %s & %s & %s) to modem...", "AT+QCFG=\"nwscanseq\",02", "AT+QCFG=\"nwscanmode\",3,0", "AT+QCFG=\"iotopmode\",0,1", "AT+CFUN=1,1");
	
		at->flush(); // flush existing data
		//at->set_debug(1);
		at->lock();
		at->cmd_start("AT+QCFG=\"nwscanseq\",02,1");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		at->cmd_start("AT+QCFG=\"nwscanmode\",3,1");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		at->cmd_start("AT+QCFG=\"iotopmode\",0,1");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		at->cmd_start("AT+CFUN=1,1");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		ret = at->unlock_return_error();
		device->release_at_handler(at);
    }
	else
	{
		CELLULAREXT_ERR("SetCatM1: get at handler fail");
		ret = -1;
	}
	
	printf("CellularExt_SetCatM1: wait for 10 secs\n");
	wait(10);
	return ret;
}

int CellularExt_SetAutoCatM1Nbiot(void)
{	
	int ret = -1;

	printf("CellularExt_SetAutoCatM1Nbiot\n");

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("SetAuto: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {

		CELLULAREXT_DEBUG("sending (%s & %s & %s & %s) to modem...", "AT+QCFG=\"nwscanseq\",00", "AT+QCFG=\"nwscanmode\",0,0", "AT+QCFG=\"iotopmode\",2,1", "AT+CFUN=1,1");
	
		at->flush(); // flush existing data
		//at->set_debug(1);
		at->lock();
		at->cmd_start("AT+QCFG=\"nwscanseq\",00,1");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		at->cmd_start("AT+QCFG=\"nwscanmode\",0,1");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		at->cmd_start("AT+QCFG=\"iotopmode\",2,1");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		at->cmd_start("AT+CFUN=1,1");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		ret = at->unlock_return_error();
		device->release_at_handler(at);
    }
	else
	{
		CELLULAREXT_ERR("SetAuto: get at handler fail");
		ret = -1;
	}
	
	printf("CellularExt_SetAutoCatM1Nbiot: wait for 10 secs\n");
	wait(10);
	return ret;
}

int CellularExt_GetBand_BC95G(uint8_t *band, uint32_t size, uint8_t *bandcnt)
{	
	int ret = -1;
	int idx, strlen;
	uint32_t strsize, i, totransfer;
	uint8_t tmp;
	char strbuff[50];
	bool valuestart;

	*bandcnt = 0;
	
	if(!band || !size)
		return -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("GetBand: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

	idx = 0;
	memset(strbuff, 0, sizeof(strbuff));
    if (at) {
		CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+NBAND?");
	
		at->flush(); // flush existing data
		//at->set_debug(1);
		at->lock();
		at->cmd_start("AT+NBAND?");
		at->cmd_stop();
		at->resp_start("+NBAND:");
	    //strlen = at->read_string(strbuff, sizeof(strbuff)-1);
	    for(i=0; i<6; i++)
	    {
	    	//band[idx++] = at->read_int();
	    	ret = at->read_int();
			if(ret <= 0)
				break;
			else
				band[idx++] = (uint8_t)ret;
	    }
		at->resp_stop();
		ret = at->unlock_return_error();
		printf("result: %d\n", ret);
		device->release_at_handler(at);
		
		//for(i=0; i<idx; i++)
		//{
		//	if(band[i] == 0xFF)
		//		break;
		//}
		//*bandcnt = i;
		*bandcnt = idx;

		/*
		// transfer string to int
		printf("read[%d]: %s\n", strlen, strbuff);
		valuestart = true;
		memset(band, 0, size);
		for(i=0; i<strlen; i++)
		{
			printf("\n[%d] %c ==> ", i, strbuff[i]);
			if((strbuff[i] == '\r') || (strbuff[i] == '\n'))
			{
				if(valuestart == false)
				{
					printf("idx++ ==> ");
					idx++;
				}
				printf("break\n");
				break;
			}
			if(strbuff[i] == ',')
			{
				printf(",");
				valuestart = true;
			}
			else
			{
				if(valuestart == false)
				{
					printf("band[%d]=%d*10 ==> ", idx, band[idx]);
					band[idx] = band[idx]*10;
					printf("%d ==> ", band[idx]);
				}
				
				if( (strbuff[i] > '0') && (strbuff[i] <= '9') )
				{
					printf("%d+%d ==> ", band[idx], (strbuff[i] - '0'));
					band[idx] += strbuff[i] - '0';
					printf("%d", band[idx]);
				}

				if(valuestart == true)
				{
					printf("valuestart=false ==> ");
					valuestart = false;
				}
				else
				{
					printf("idx++ ==> valuestart=start ==> ");
					idx++;
					valuestart = true;
				}
			}
		}
		*bandcnt = idx;
		printf("\nbandcnt = %d\n");
		*/
		printf("bandcnt = %d\n", *bandcnt);
    }
	else
	{
		CELLULAREXT_ERR("GetBand: get at handler fail");
		ret = -1;
	}

	return ret;
}

int CellularExt_GetBand_R410M(uint8_t *band, uint32_t size, uint8_t *bandcnt)
{	
	uint64_t ret;
	int i, idx, retry;
	uint8_t tmp;
	uint8_t rat[2];
	uint64_t bandmask[2];

	*bandcnt = 0;
	retry = 0;
	
	if(!band || !size)
		return -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("GetBand: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
_GET_BAND:
		CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+UBANDMASK?");
		idx = 0;
		at->flush(); // flush existing data
		at->set_debug(1);
		at->lock();
		at->cmd_start("AT+UBANDMASK?");
		at->cmd_stop();
		at->resp_start("+UBANDMASK:");
	    for(i=0; i<2; i++)
	    {
			rat[idx] = (uint8_t)at->read_int();
			bandmask[idx++] = (uint64_t)at->read_int();
	    }
		at->resp_stop();
		at->set_debug(0);
		ret = at->unlock_return_error();
		printf("result: %d\n", ret);
		device->release_at_handler(at);

		printf("support band mask: [%d]%lld, [%d]%lld\n", rat[0], bandmask[0], rat[1], bandmask[1]);
		for(i=0; i<2; i++)
		{
			if(rat[i] == 1)	// RAT: 0=catm1, 1=nbiot
				break;
		}
		if(i >= 2)
		{
			printf("no bandmask for nbiot\n");

			CELLULAREXT_DEBUG("sending (%s + %s) to modem...", "at+umnoprof=100", "at+cfun=15");
			at->flush(); // flush existing data
			at->set_debug(1);
			at->lock();
			at->cmd_start("at+umnoprof=100");
			at->cmd_stop_read_resp();
			at->cmd_start("at+cfun=15");
			at->cmd_stop_read_resp();
			ret = at->unlock_return_error();
			printf("result: %d\n", ret);
			at->set_debug(0);
			device->release_at_handler(at);
			wait(5);
			retry++;
			if(retry <= 1)
			{
				wait(2);
				goto _GET_BAND;
			}
			else
				return -1;
		}

		tmp = i;
		idx = 0;
		printf("rat:%d, bandmask\n", rat[tmp], bandmask[tmp]);
		if(bandmask[tmp] > 0)
		{
			for(i=0; i<64; i++)
			{
				if(bandmask[tmp] & (1<<i))
				{
					band[idx++] = i+1;	// bit0 = band1, bit1 = band2, ....
					if(idx >= size)
					{
						printf("support band over count !!!\n", ret);
						break;
					}
				}
			}
		}
		*bandcnt = idx;
		printf("bandcnt = %d\n", *bandcnt);
    }
	else
	{
		CELLULAREXT_ERR("GetBand: get at handler fail");
		ret = -1;
	}

	return ret;
}

int CellularExt_SetBand_BC95G(uint8_t *band, uint32_t size)
{	
	int ret = -1;
	int i;
	int offset;
	char cmdstr[50];

	if(!band || !size)
		return -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("SetBand: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		offset = 0;
		memset(cmdstr, 0, sizeof(cmdstr));
		offset += snprintf(cmdstr, sizeof(cmdstr), "AT+NBAND=");
		for(i=0; i<size; i++)
		{
			offset += snprintf(&cmdstr[offset], sizeof(cmdstr)-offset, "%d,", band[i]);
		}
		cmdstr[offset-1] = 0;
	
		CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+CFUN=0");
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CFUN=0");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		at->unlock_return_error();

		wait(3);
		CELLULAREXT_DEBUG("sending (%s) to modem...", cmdstr);		
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start(cmdstr);
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		at->unlock_return_error();

		wait(3);
		CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+CFUN=1");		
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CFUN=1");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		ret = at->unlock_return_error();
		
		device->release_at_handler(at);
    }
	else
	{
		CELLULAREXT_ERR("SetBand: get at handler fail");
		ret = -1;
	}

	return ret;
}

int CellularExt_SetBand_R410M(uint8_t *band, uint32_t size)
{	
	int ret = -1;
	int i;
	int retry = 0;
	int offset;
	char cmdstr[50];
	uint64_t bandmask;

	if(!band || !size)
		return -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("SetBand: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		offset = 0;
		memset(cmdstr, 0, sizeof(cmdstr));
		offset += snprintf(cmdstr, sizeof(cmdstr), "AT+UBANDMASK=1,");
		bandmask = 0;
		for(i=0; i<size; i++)
		{
			bandmask |= 1<<(band[i]-1);
		}
		offset += snprintf(&cmdstr[offset], sizeof(cmdstr)-offset, "%lld,", bandmask);
		cmdstr[offset-1] = 0;

		/* // no need
		CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+CFUN=0");
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CFUN=0");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		ret = at->unlock_return_error();
		wait(3);
		*/
		
__SET_BAND:
	
		/* // no need
		CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+URAT=8");
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+URAT=8");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		ret = at->unlock_return_error();
		wait(1);
		*/		

		//CELLULAREXT_DEBUG("sending ([%d]%s) to modem...", strlen("AT+UBANDMASK=1,134217860"), "AT+UBANDMASK=1,134217860");
		CELLULAREXT_DEBUG("sending ([%d]%s) to modem...", strlen((const char *)cmdstr), cmdstr);
		at->flush(); // flush existing data
		at->set_debug(1);
		at->lock();
		//at->cmd_start("AT+UBANDMASK=1,134217860"); //134217860 = 3+8+28, //185538719"); //
		at->cmd_start(cmdstr);
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		ret = at->unlock_return_error();
		wait(1);
		if( (ret != 0) && (retry < 3) )
		{
			retry++;
			wait(2);
			goto __SET_BAND;
		}

		/* // no need
		CELLULAREXT_DEBUG("sending (%s) to modem...", "AT+CFUN=15");
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CFUN=15");
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		ret = at->unlock_return_error();
		wait(5);
		*/

		at->set_debug(0);
		device->release_at_handler(at);
    }
	else
	{
		CELLULAREXT_ERR("SetBand: get at handler fail");
		ret = -1;
	}

	return ret;
}

int CellularExt_ScramblingEn(bool en)
{
	int ret = -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("ScramblingEn: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		if(en)
			CELLULAREXT_DEBUG("sending (%s & %s) to modem...", "AT+NCONFIG:CR_0354_0338_SCRAMBLING,TRUE", "AT+NCONFIG:CR_0859_SI_AVOID,TRUE");
		else
			CELLULAREXT_DEBUG("sending (%s & %s) to modem...", "AT+NCONFIG:CR_0354_0338_SCRAMBLING,FALSE", "AT+NCONFIG:CR_0859_SI_AVOID,FALSE");

		at->flush(); // flush existing data
		//at->set_debug(1);
		at->lock();
		if(en)
		{
			at->cmd_start("AT+NCONFIG:CR_0354_0338_SCRAMBLING,TRUE");
			at->cmd_stop();
			at->resp_start();
			at->resp_stop();

			at->cmd_start("AT+NCONFIG:CR_0859_SI_AVOID,TRUE");
			at->cmd_stop();
			at->resp_start();
			at->resp_stop();
		}
		else
		{
			at->cmd_start("+NCONFIG:CR_0354_0338_SCRAMBLING,FALSE");
			at->cmd_stop();
			at->resp_start();
			at->resp_stop();
			
			at->cmd_start("+NCONFIG:CR_0859_SI_AVOID,FALSE");
			at->cmd_stop();
			at->resp_start();
			at->resp_stop();
		}
		ret = at->unlock_return_error();
		device->release_at_handler(at);
    }
	else
	{
		CELLULAREXT_ERR("ScramblingEn: get at handler fail");
		ret = -1;
	}

	wait(0.3);
	return ret;
}


int CellularExt_SetAPN(char *apn)
{	
	int ret = -1;

	if(!apn)
		return -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("SetAPN: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		at->set_debug(1);
		
		printf("\nsending (%s%s) to modem...\n", "AT+CGDCONT=1,IP,", apn);
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CGDCONT=");
		at->write_int(1); // cid
		at->write_string("IP");
		at->write_string(apn);
		at->cmd_stop_read_resp();
		ret = at->unlock_return_error();

		printf("\nsending (%s) to modem...\n", "AT+CGDCONT?");
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CGDCONT?");
		at->cmd_stop_read_resp();
		ret = at->unlock_return_error();

		at->set_debug(0);
		device->release_at_handler(at);
    }
	else
	{
		CELLULAREXT_ERR("SetAPN: get at handler fail");
		ret = -1;
	}

	wait(0.3);
	return ret;
}

int CellularExt_ClearAPN(void)
{
	int ret = -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("SetAPN: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		at->set_debug(1);

		printf("sending (%s) to modem...\n", "AT+CGDCONT?");
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CGDCONT?");
		at->cmd_stop_read_resp();
		ret = at->unlock_return_error();
		
		printf("sending (%s) to modem...\n", "AT+CGDCONT=1");
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CGDCONT=1");
		at->cmd_stop_read_resp();
		ret = at->unlock_return_error();

		printf("sending (%s) to modem...\n", "AT+CGDCONT=2");
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CGDCONT=2");
		at->cmd_stop_read_resp();
		ret = at->unlock_return_error();

		at->set_debug(0);
		device->release_at_handler(at);
    }
	else
	{
		CELLULAREXT_ERR("SetAPN: get at handler fail");
		ret = -1;
	}

	wait(0.3);
	return ret;
}

int CellularExt_QueryAPN(void)
{	
	int ret = -1;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("QueryAPN: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		at->set_debug(1);

		printf("\nsending (%s) to modem...\n", "AT+CGDCONT?");
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CGDCONT?");
		at->cmd_stop_read_resp();
		ret = at->unlock_return_error();

		//at->set_debug(0);
		device->release_at_handler(at);
    }
	else
	{
		CELLULAREXT_ERR("QueryAPN: get at handler fail");
		ret = -1;
	}

	wait(0.3);
	return ret;
}

int CellularExt_GetPsm(bool *psm)
{
	int retry;
	int ret = -1;
	int mode;
	//int req_periodic_rau;
	//int req_gprs_ready_timer;
	//int req_periodic_tau;
	//int req_active_time;

	*psm = true;
	retry = 0;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("GetPsm: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
_SEND_CMD:
		printf("sending (%s) to modem...\n", "AT+CPSMS?");
		//at->set_debug(1);
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CPSMS?");
		//at->cmd_stop_read_resp();
		at->resp_start("+CPSMS:");
		mode = at->read_int();
		//req_periodic_rau = at->read_int();
		//req_gprs_ready_timer = at->read_int();
		//req_periodic_tau = at->read_int();
		//req_active_time = at->read_int();
		at->resp_stop();
		ret = at->unlock_return_error();

		if( (ret<0) && (retry<2) )
		{
			retry++;
			wait(2);
			goto _SEND_CMD;
		}
		
		device->release_at_handler(at);
		//at->set_debug(0);
		
		*psm = (mode)? true:false;
		//printf("GetPsm: mode:%d, req_periodic_rau:%d, req_gprs_ready_timer:%d, req_periodic_tau:%d, req_active_time:%d\n", 
		//		mode, req_periodic_rau, req_gprs_ready_timer, req_periodic_tau, req_active_time);
		printf("GetPsm: mode:%d\n", mode);
    }
	else
	{
		CELLULAREXT_ERR("GetPsm: get at handler fail");
		ret = -1;
	}

	wait(0.3);
	return ret;
}

int CellularExt_GetEdrx(bool *edrx)
{
	int ret = -1;
	int mode;
	int req_edrx_value;

	*edrx = false;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("GetEdrx: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		printf("sending (%s) to modem...\n", "AT+CEDRXS?");
		//at->set_debug(1);
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CEDRXS?");
		//at->cmd_stop_read_resp();
		at->resp_start("+CEDRXS:");
		mode = at->read_int();
		req_edrx_value = at->read_int();
		at->resp_stop();
		ret = at->unlock_return_error();
		
		device->release_at_handler(at);
		//at->set_debug(0);
		
		*edrx = (mode>0)? true:false;
		printf("GetEdrx: mode:%d, req_edrx_value:%d\n", mode, req_edrx_value);

    }
	else
	{
		CELLULAREXT_ERR("GetEdrx: get at handler fail");
		ret = -1;
	}

	wait(0.3);
	return ret;
}

int CellularExt_GetEdrxrdp(void)
{
	int ret = -1;
	char buff[100];

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("GetEdrxrdp: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		printf("sending (%s) to modem...\n", "AT+CEDRXRDP");
		//at->set_debug(1);
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CEDRXRDP");
		//at->cmd_stop_read_resp();
		at->resp_start("+CEDRXRDP:");
		at->read_string(buff, sizeof(buff));
		at->resp_stop();
		ret = at->unlock_return_error();
		
		device->release_at_handler(at);
		//at->set_debug(0);
    }
	else
	{
		CELLULAREXT_ERR("GetEdrxrdp: get at handler fail");
		ret = -1;
	}

	wait(0.3);
	return ret;
}

static int CellularExt_DisablePsm_R410M(void)
{
	int ret = -1;
	int retry = 0;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("DisablePsm: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		at->set_debug(1);
__RETRY:
		printf("sending (%s) to modem...\n", "AT+CPSMS=0");
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CPSMS=0");
		at->cmd_stop_read_resp();
		ret = at->unlock_return_error();
		if(ret < 0)
		{
			retry++;
			if(retry < 3)
			{
				wait(2);
				goto __RETRY;
			}
		}
		
		device->release_at_handler(at);
		at->set_debug(0);
    }
	else
	{
		CELLULAREXT_ERR("DisablePsm: get at handler fail");
		ret = -1;
	}

	wait(0.3);
	return ret;
}

static int CellularExt_EnablePsmNotice_R410M(void)
{
	int ret = -1;
	int retry = 0;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("EnablePsmNotice: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		at->set_debug(1);
__RETRY:
		printf("sending (%s) to modem...\n", "AT+UPSMR=1");
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+UPSMR=1");
		at->cmd_stop_read_resp();
		ret = at->unlock_return_error();
		if(ret < 0)
		{
			retry++;
			if(retry < 3)
			{
				wait(2);
				goto __RETRY;
			}
		}
		
		device->release_at_handler(at);
		at->set_debug(0);
    }
	else
	{
		CELLULAREXT_ERR("EnablePsmNotice: get at handler fail");
		ret = -1;
	}

	wait(0.3);
	return ret;
}

static int CellularExt_EPSNetworkRegistrationStatus(void)
{
	int ret = -1;
	int retry = 0;

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("EPSNetworkRegistrationStatus: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		at->set_debug(1);
__RETRY:
		// 2:  network  registration  and  location  information  URC  +CEREG:  <stat>[,[<tac>],[<ci>],[<AcT>]] enabled
		printf("sending (%s) to modem...\n", "AT+CEREG=2");
		at->flush(); // flush existing data
		at->lock();
		at->cmd_start("AT+CEREG=2");
		at->cmd_stop_read_resp();
		ret = at->unlock_return_error();
		if(ret < 0)
		{
			retry++;
			if(retry < 3)
			{
				wait(2);
				goto __RETRY;
			}
		}
		
		device->release_at_handler(at);
		at->set_debug(0);
    }
	else
	{
		CELLULAREXT_ERR("EPSNetworkRegistrationStatus: get at handler fail");
		ret = -1;
	}

	wait(0.3);
	return ret;
}



//--------------------------------------------------------------------//
// hw control
//--------------------------------------------------------------------//
#define _MODEM_HWRESET_RESET
#define _MODEM_HWRESET_SET

#if defined(_MODEM_HWRESET_RESET) || defined(_MODEM_HWRESET_SET)
/*
PullNone		  = 0,
PullUp			  = 1,
PullDown		  = 2,
OpenDrainPullUp   = 3,
OpenDrainNoPull   = 4,
OpenDrainPullDown = 5,
*/
// for bg96, PullNone-->3.3v, OpenDrainPullUp-->2.45v not enough
#define MDMRESET_TYPE		PullNone
DigitalInOut modem_hwreset(MDMRESET, PIN_OUTPUT, MDMRESET_TYPE, 0); // default 1 for bg96, 0 for bc95g
#endif

static void CellularExt_HWReset_ResetDownPulse(void)
{
	#ifdef _MODEM_HWRESET_RESET

	#ifdef _DEVICE_UBLOX_SARA_R410M
	#define MDMRESET_PULSE	11		// min 10s for power down
	printf("modem hw reset down pulse %d secs\n", MDMRESET_PULSE);
	#else
	#define MDMRESET_PULSE	0.3		// >100ms for bc95g, 150~460ms for bg96	
	printf("modem hw reset down pulse %f secs\n", MDMRESET_PULSE);
	#endif
		
	modem_hwreset = 0;
	wait(MDMRESET_PULSE);
	modem_hwreset = 1;
	printf("and wait for 15s\n");
	wait(15);
	printf("---\n");
	#endif
}

static void CellularExt_HWReset_ResetUpPulse(void)
{
	#ifdef _MODEM_HWRESET_RESET	
	#ifdef _DEVICE_UBLOX_SARA_R410M	
	#define MDMRESET_PULSE	11		// min 10s for power down
	printf("modem hw reset down pulse %d secs\n", MDMRESET_PULSE);
	#else
	#define MDMRESET_PULSE	0.3 	// >100ms for bc95g, 150~460ms for bg96 
	printf("modem hw reset down pulse %f secs\n", MDMRESET_PULSE);
	#endif

	modem_hwreset = 1;
	wait(MDMRESET_PULSE);
	modem_hwreset = 0;
	printf("and wait for 15s\n");
	wait(15);
	printf("---\n");
	#endif
}

void CellularExt_HWReset_Reset(void)
{
	printf("cellular hw reset\n");
	
	#if defined(_DEVICE_QUECTEL_BC95)
	CellularExt_HWReset_ResetUpPulse();
	#elif defined(_DEVICE_UBLOX_SARA_R410M)
	printf("SARA-R410M RESET_N can only power off, ignore");
	#else
	CellularExt_HWReset_ResetDownPulse();
	#endif
}

void CellularExt_HWReset_Set(int high)
{
	#ifdef _MODEM_HWRESET_SET
	high = (high>0)? 1 : 0;
	printf("set modem reset pin (type=%d) %d\n", MDMRESET_TYPE, high);
	modem_hwreset = high;
	#endif
}

int CellularExt_SWReset_Reset(void)
{
	int ret = -1;

	printf("cellular sw reset\n");

	#if defined(_DEVICE_QUECTEL_BC95) || defined(_DEVICE_QUECTEL_BG96)
	const char cmd[] = "AT+CFUN=1,1";
	#elif defined(_DEVICE_UBLOX_SARA_R410M)
	const char cmd[] = "AT+CFUN=15";
	#else
	printf("not defined cellular device, return\n");
	#endif

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("SWReset_Reset: get cellular device fail");
		return -1;
	}
    ATHandler *at = device->get_at_handler();

    if (at) {
		CELLULAREXT_DEBUG("sending (%s) to modem...", cmd);
		
		at->flush(); // flush existing data
		//at->set_debug(1);
		at->lock();
		at->cmd_start(cmd);
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		ret = at->unlock_return_error();
		device->release_at_handler(at);
    }
	else
	{
		CELLULAREXT_ERR("SWReset_Reset: get at handler fail");
		ret = -1;
	}

	printf("SWReset_Reset: wait for 5 secs\n");
	wait(5);
	return ret;
}

//--------------------------------------------------------------------//
// specific config for custom cell module
//--------------------------------------------------------------------//
int CellularExt_PreInitModule(void)
{
	// config module for specific
	int i, j, k, x, y, matchcnt;
	char strbuff[50];
	uint8_t band[20];
	uint8_t bandcnt;
	uint8_t bandmask[3];
	bool psm;
	int ret;

	printf("CellularExt_PreInitModule\n");
	CellularExt.module = CELLMODULE_MAX;

	for(i=0; i<2; i++)
	{
		for(j=0; j<5; j++)
		{
			memset(strbuff, 0, sizeof(strbuff));
			CellularExt_GetModelIdentification(strbuff, sizeof(strbuff));

			for(k=0; k<CELLMODULE_MAX; k++)
			{
				if(strstr(strbuff, ModuleNameList[k]) != NULL)
					break;
			}
			switch(k)
			{
				case CELLMODULE_BG96:
					printf("BG96 detected\n");
					CellularExt.module = CELLMODULE_BG96;

					// config mode if necessary
					#if defined(_CELLULAR_DEVICE_NBIOT)
					CellularExt_SetNbiot();
					#elif defined(_CELLULAR_DEVICE_CATM1)
					CellularExt_SetCatM1();
					#elif defined(_CELLULAR_DEVICE_AUTO)
					CellularExt_SetAutoCatM1Nbiot();
					#endif

					// enable gps
					//CellularExt_GPSEn(true);

					// set apn if necessary
					#if MBED_CONF_NSAPI_DEFAULT_CELLULAR_APN
					CellularExt_SetAPN(MBED_CONF_NSAPI_DEFAULT_CELLULAR_APN);
					#else
					//CellularExt_ClearAPN();
					#endif
					break;
				case CELLMODULE_BC95G:
					printf("BC95-G detected\n");
					CellularExt.module = CELLMODULE_BC95G;
					// get band
					CellularExt_GetBand_BC95G(band, sizeof(band), &bandcnt);
					printf("support band[%d]: ", bandcnt); // default: +NBAND:5,8,3,28,20,1
					for(int a=0; a<bandcnt; a++)
						printf("%d ", band[a]);
					printf("\n");
					// set band if necessary
					#if defined(_CELLULAR_DEVICE_BAND28)		// for specify band 28 only
					if( (bandcnt > 1) || band[0] != 28)
					{
						// set band 28
						band[0] = 28;
						CellularExt_SetBand_BC95G(band, 1);
						
						CellularExt_GetBand_BC95G(band, sizeof(band), &bandcnt);
						printf("support band[%d]: ", bandcnt);
						for(int a=0; a<bandcnt; a++)
							printf("%d ", band[a]);
						printf("\n");

						return 1;
					}
					#elif defined(_CELLULAR_DEVICE_BAND3_8_28)	// for specify band 3,8,28 only
					bandmask[0] = 3;
					bandmask[1] = 8;
					bandmask[2] = 28;
					matchcnt = 0;
					for(x=0; x<3; x++)
					{
						for(y=0; y<3; y++)
						{
							if(band[x] == bandmask[y])
								matchcnt++;
						}
					}
					if( (bandcnt != 3) || (matchcnt != 3) )
					{
						// set band 3,8,28
						band[0] = 3;
						band[1] = 8;
						band[2] = 28;
						CellularExt_SetBand_BC95G(band, 3);
						
						CellularExt_GetBand_BC95G(band, sizeof(band), &bandcnt);
						printf("support band[%d]: ", bandcnt);
						for(int a=0; a<bandcnt; a++)
							printf("%d ", band[a]);
						printf("\n");

						return 1;
					}
					#elif defined(_CELLULAR_DEVICE_BANDFULL)	// for specify full band
					if(bandcnt < 6)
					{
						// set full band
						band[0] = 5;
						band[1] = 8;
						band[2] = 3;
						band[3] = 28;
						band[4] = 20;
						band[5] = 1;
						CellularExt_SetBand_BC95G(band, 6);
						
						CellularExt_GetBand_BC95G(band, 10, &bandcnt);
						printf("support band[%d]: ", bandcnt);
						for(int a=0; a<bandcnt; a++)
							printf("%d ", band[a]);
						printf("\n");

						return 1;
					}
					#endif
					
					// set apn if necessary
					#if MBED_CONF_NSAPI_DEFAULT_CELLULAR_APN
					CellularExt_SetAPN(MBED_CONF_NSAPI_DEFAULT_CELLULAR_APN);
					#else
					//CellularExt_ClearAPN();
					#endif
					break;
				case CELLMODULE_R410M:
					printf("R410M detected\n");
					CellularExt.module = CELLMODULE_R410M;
					// get band
					CellularExt_GetBand_R410M(band, sizeof(band), &bandcnt);
					printf("support band[%d]: ", bandcnt); // default: +NBAND:5,8,3,28,20,1
					for(int a=0; a<bandcnt; a++)
						printf("%d ", band[a]);
					printf("\n");
					// set band if necessary
					#if defined(_CELLULAR_DEVICE_BAND28)		// for specify band 28 only
					if( (bandcnt > 1) || band[0] != 28)
					{
						// set band 28
						band[0] = 28;
						CellularExt_SetBand_R410M(band, 1);
						
						CellularExt_GetBand_R410M(band, sizeof(band), &bandcnt);
						printf("support band[%d]: ", bandcnt);
						for(int a=0; a<bandcnt; a++)
							printf("%d ", band[a]);
						printf("\n");
					}
					#elif defined(_CELLULAR_DEVICE_BAND3_8_28)	// for specify band 3,8,28 only
					bandmask[0] = 3;
					bandmask[1] = 8;
					bandmask[2] = 28;
					matchcnt = 0;
					for(x=0; x<3; x++)
					{
						for(y=0; y<3; y++)
						{
							if(band[x] == bandmask[y])
								matchcnt++;
						}
					}
					if( (bandcnt != 3) || (matchcnt != 3) )
					{
						// set band 3,8,28
						band[0] = 3;
						band[1] = 8;
						band[2] = 28;
						CellularExt_SetBand_R410M(band, 3);
						
						CellularExt_GetBand_R410M(band, sizeof(band), &bandcnt);
						printf("support band[%d]: ", bandcnt);
						for(int a=0; a<bandcnt; a++)
							printf("%d ", band[a]);
						printf("\n");
					}
					#elif defined(_CELLULAR_DEVICE_BANDFULL)	// for specify full band
					if(bandcnt < 14)
					{
						// set full band
            			band[0] = 1;
						band[1] = 2;
						band[2] = 3;
						band[3] = 4;
						band[4] = 5;
						band[5] = 8;
						band[6] = 12;
						band[7] = 13;
						band[8] = 17;
						band[9] = 18;
						band[10] = 19;
						band[11] = 20;
						//band[12] = 25; // no band25 , it will ERROR
						band[12] = 26;
						band[13] = 28;
						CellularExt_SetBand_R410M(band, 14);
						
						CellularExt_GetBand_R410M(band, sizeof(band), &bandcnt);
						printf("support band[%d]: ", bandcnt);
						for(int a=0; a<bandcnt; a++)
							printf("%d ", band[a]);
						printf("\n");
					}
					#endif

					//CellularExt_ClearAPN();
					//CellularExt_SetAPN("internet.iot");
					//CellularExt_QueryAPN();

					// disable psm
					//ret = CellularExt_GetPsm(&psm);
					//printf("psm-preset: %d\n", (ret<0)? ret:psm);
					CellularExt_EnablePsmNotice_R410M();
					CellularExt_DisablePsm_R410M();
					//CellularExt_EPSNetworkRegistrationStatus();
					/*
					ret = CellularExt_GetPsm(&psm);
					printf("psm-set1: %d\n", (ret<0)? ret:psm);
					for(i=2; i<=10; i++)
					{
						if((!ret) || !psm)
							break;
						else
						{
							CellularExt_DisablePsm_R410M();
							ret = CellularExt_GetPsm(&psm);
							printf("psm-set%d: %d\n", i, (ret<0)? ret:psm);
						}
					}
					*/
					//CellularExt_EnableSignalStrengthReport();
					break;

				case CELLMODULE_N410:
					printf("N410 detected\n");
					CellularExt.module = CELLMODULE_N410;
					// get band
					CellularExt_GetBand_R410M(band, sizeof(band), &bandcnt);
					printf("support band[%d]: ", bandcnt); // default: +NBAND:5,8,3,28,20,1
					for(int a=0; a<bandcnt; a++)
						printf("%d ", band[a]);
					printf("\n");
					// set band if necessary
					#if defined(_CELLULAR_DEVICE_BAND28)		// for specify band 28 only
					if( (bandcnt > 1) || band[0] != 28)
					{
						// set band 28
						band[0] = 28;
						CellularExt_SetBand_R410M(band, 1);
						
						CellularExt_GetBand_R410M(band, sizeof(band), &bandcnt);
						printf("support band[%d]: ", bandcnt);
						for(int a=0; a<bandcnt; a++)
							printf("%d ", band[a]);
						printf("\n");
					}
					#elif defined(_CELLULAR_DEVICE_BAND3_8_28)	// for specify band 3,8,28 only
					bandmask[0] = 3;
					bandmask[1] = 8;
					bandmask[2] = 28;
					matchcnt = 0;
					for(x=0; x<3; x++)
					{
						for(y=0; y<3; y++)
						{
							if(band[x] == bandmask[y])
								matchcnt++;
						}
					}
					if( (bandcnt != 3) || (matchcnt != 3) )
					{
						// set band 3,8,28
						band[0] = 3;
						band[1] = 8;
						band[2] = 28;
						CellularExt_SetBand_R410M(band, 3);
						
						CellularExt_GetBand_R410M(band, sizeof(band), &bandcnt);
						printf("support band[%d]: ", bandcnt);
						for(int a=0; a<bandcnt; a++)
							printf("%d ", band[a]);
						printf("\n");
					}
					#elif defined(_CELLULAR_DEVICE_BANDFULL)	// for specify full band
					if(bandcnt < 14)
					{
						// set full band
            			band[0] = 1;
						band[1] = 2;
						band[2] = 3;
						band[3] = 4;
						band[4] = 5;
						band[5] = 8;
						band[6] = 12;
						band[7] = 13;
						band[8] = 17;
						band[9] = 18;
						band[10] = 19;
						band[11] = 20;
						//band[12] = 25; // no band25 , it will ERROR
						band[12] = 26;
						band[13] = 28;
						CellularExt_SetBand_R410M(band, 14);
						
						CellularExt_GetBand_R410M(band, sizeof(band), &bandcnt);
						printf("support band[%d]: ", bandcnt);
						for(int a=0; a<bandcnt; a++)
							printf("%d ", band[a]);
						printf("\n");
					}
					#endif
					//CellularExt_EnableSignalStrengthReport();
					break;
					
				default:
					printf("unknow detected: \n---\n%s\n---\n", strbuff);
					break;
			}
			if(k < CELLMODULE_MAX)
				break;
		}
		
		if(k < CELLMODULE_MAX)
			break;

		//CellularExt_HWReset_Reset(); // remove to outside to choose if doing hw reset
	}

	if(k < CELLMODULE_MAX)
	{
		char fwver[50];
		CellularExt_GetFwVersion(fwver, sizeof(fwver));
		printf("fw version: %s\n", fwver);

		return 0;
	}
	else
		return -1;

	//if(k < CELLMODULE_MAX)
	//	return 0;
	//else
	// 	return -1;
}


void CellularExt_DisconnectRAI(void)
{
	if(CellularExt.module == CELLMODULE_BC95G)
	{
		int ret = -1;
		int32_t socket;
		const char cmd1[] = "AT+NSOCR=DGRAM,17,7";
		const char cmd2_1[] = "AT+NSOSTF=";
		const char cmd2_2[] = ",13.76.154.69,5131,0x200,1,AA,1";	// {13, 76, 154, 69} 5131
		const char cmd3[] = "AT+NSOCL=";

		char cmd2[50];

	    // take the ATHandler into use for sending ad hoc commands to the modem
	    CellularDevice *device = CellularDevice::get_default_instance();
		if (!device)
		{
			CELLULAREXT_ERR("DisableRAI: get cellular device fail");
			return;
		}
	    ATHandler *at = device->get_at_handler();

	    if (at) {
			at->flush(); // flush existing data
			//at->set_debug(1);

			CELLULAREXT_DEBUG("sending (%s) to modem...", cmd1);
			at->lock();
			at->cmd_start(cmd1);
			at->cmd_stop();			
			at->set_delimiter(0);
			at->resp_start();
			socket = at->read_int();
			at->resp_stop();
			at->set_default_delimiter();
			ret = at->unlock_return_error();
			CELLULAREXT_DEBUG("result: %d", ret);			
			if(ret != 0)
			{
				CELLULAREXT_ERR("result: %d", ret);
			}

			memset(cmd2, 0, sizeof(cmd2));
			snprintf(cmd2, sizeof(cmd2), "%s%d%s", cmd2_1, socket, cmd2_2);
			CELLULAREXT_DEBUG("sending (%s) to modem...", cmd2);
			at->lock();
			at->cmd_start(cmd2);
			at->cmd_stop();
			at->set_delimiter(0);
			at->resp_start();
			at->resp_stop();
			at->set_default_delimiter();
			ret = at->unlock_return_error();
			CELLULAREXT_DEBUG("result: %d", ret);
			if(ret != 0)
			{
				CELLULAREXT_ERR("result: %d", ret);
			}

			CELLULAREXT_DEBUG("sending (%s) to modem...", cmd3);
			at->lock();
			at->cmd_start(cmd3);
			at->write_int(socket);
			at->cmd_stop();			
			at->set_delimiter(0);
			at->resp_start();
			at->resp_stop();
			at->set_default_delimiter();
			ret = at->unlock_return_error();
			CELLULAREXT_DEBUG("result: %d", ret);
			if(ret != 0)
			{
				CELLULAREXT_ERR("result: %d", ret);
			}
			
			device->release_at_handler(at);
	    }
		else
			CELLULAREXT_ERR("DisableRAI: get at handler fail");
	}
	else if(CellularExt.module == CELLMODULE_BG96)
	{
		int ret = -1;
		int32_t socket;
		const char cmd1[] = "AT+QCFG=\"rrcabort\",1";
		
	    // take the ATHandler into use for sending ad hoc commands to the modem
	    CellularDevice *device = CellularDevice::get_default_instance();
		if (!device)
		{
			CELLULAREXT_ERR("DisableRAI: get cellular device fail");
			return;
		}
	    ATHandler *at = device->get_at_handler();

	    if (at) {
			at->flush(); // flush existing data
			//at->set_debug(1);

			CELLULAREXT_DEBUG("sending (%s) to modem...", cmd1);
			at->lock();
			at->cmd_start(cmd1);
			at->cmd_stop();			
			at->set_delimiter(0);
			at->resp_start();
			at->resp_stop();
			at->set_default_delimiter();
			ret = at->unlock_return_error();
			CELLULAREXT_DEBUG("result: %d", ret);
			if(ret != 0)
			{
				CELLULAREXT_ERR("result: %d", ret);
			}
			
			device->release_at_handler(at);
	    }
		else
			CELLULAREXT_ERR("DisableRAI: get at handler fail");
	}
}

void CellularExt_Attach(void)
{
	int i;
	int stat;
	int ret;

	if(CellularExt.module == CELLMODULE_MAX)
	{
		CELLULAREXT_ERR("Attach: not cellular in list, exit");
		return;
	}

	/*
	const char cmd_bc95g[] = "AT+CGATT=1";
	const char cmd_bg96[] = "AT+QIACT=1";
	const char *cmd;

	//if(CellularExt.module == CELLMODULE_BG96)
	//	cmd = cmd_bg96;
	//else
		cmd = cmd_bc95g;
	*/
	const char cmd_attachcell[] = "AT+CGATT=1";
	const char cmd_attachpdp[] = "AT+QIACT=1";
	
	// take the ATHandler into use for sending ad hoc commands to the modem
	CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("Attach: get cellular device fail");
		return;
	}
	ATHandler *at = device->get_at_handler();
	
	if (at) {
		at->set_debug(1);

		//<sophie, comfirm attached status
		printf("\nsending (%s) to modem...\n", "AT+CGATT?");
		for(i=0; i<10; i++)
		{		
			at->flush(); // flush existing data
			at->lock();
			at->cmd_start("AT+CGATT?");
			at->cmd_stop();
			at->resp_start("+CGATT:");
			stat = at->read_int();
			at->resp_stop();
			ret = at->unlock_return_error();
			if(ret == 0)
				break;
			wait(2);
		}
		//>sophie, comfirm attached

		if(stat != 1)
		{
			printf("\nsending (%s) to modem...\n", cmd_attachcell);
			for(i=0; i<10; i++)
			{
				at->flush(); // flush existing data
				at->lock();
				at->cmd_start(cmd_attachcell);
				at->cmd_stop();
				at->resp_start();
				at->resp_stop();
				ret = at->unlock_return_error();
				if(ret == 0)
					break;
				wait(2);
			}
		}
#if 0		
		//<sophie, comfirm cell registered
		printf("\nsending (%s) to modem...\n", "AT+CEREG?");
		for(i=0; i<10; i++)
		{
			at->cmd_start("AT+CEREG?");
			at->cmd_stop();
			at->resp_start("+CEREG:");
			at->read_int();	// ignore n
			stat = at->read_int();
			at->resp_stop();
			if(stat == 1)
				break;
			at->cmd_start("AT+CREG?");
			at->cmd_stop();
			at->resp_start("+CEREG:");
			at->read_int();	// ignore n
			stat = at->read_int();
			at->resp_stop();
			if(stat == 1)
				break;
		}
		//>sophie, comfirm cell registered

		//<sophie, config to specific rat if not match
		#ifdef _DEVICE_QUECTEL_BG96
		#if defined(_CELLULAR_DEVICE_NBIOT) || defined(_CELLULAR_DEVICE_CATM1)
		{
			int mode;
			int format;
			char op_long[16 + 1] = {0}; // MAX_OPERATOR_NAME_LONG = 16
			char op_short[8 + 1] = {0}; // MAX_OPERATOR_NAME_SHORT = 8
			char op_num[8 + 1] = {0};
			//"Radio access technology to use. Value in integer: GSM=0, GSM_COMPACT=1, UTRAN=2, EGPRS=3, HSDPA=4, HSUPA=5, HSDPA_HSUPA=6, E_UTRAN=7, CATM1=8 ,NB1=9",
			int op_rat = 10; //RAT_UNKNOWN;
			int specify_op_rat = 10; //RAT_UNKNOWN;
			
			at->cmd_start("AT+COPS?");
			at->cmd_stop();
			at->resp_start("+COPS:");
			mode = at->read_int();	// sophie, config to specific rat if not match //ignore mode
			format = at->read_int();
			if (at->get_last_error() == NSAPI_ERROR_OK) {
				switch (format) {
					case 0:
						at->read_string(op_long, sizeof(op_long));
						break;
					case 1:
						at->read_string(op_short, sizeof(op_short));
						break;
					default:
						at->read_string(op_num, sizeof(op_num));
						break;
				}
				op_rat = at->read_int();
			}
			at->resp_stop();

			#if defined(_CELLULAR_DEVICE_NBIOT)
			specify_op_rat = 9;
			#elif defined(_CELLULAR_DEVICE_CATM1)
			specify_op_rat = 8;
			#endif
			
			if(specify_op_rat == 10)
			{
				printf("specify_op_rat unknown %d\n", specify_op_rat);
			}
			else if(op_rat != specify_op_rat)
			{
				printf("config specific rat %d\n", specify_op_rat);
				at->cmd_start("AT+COPS=");
				at->write_int(mode);
				at->write_int(format);
				switch (format) {
					case 0:
						at->write_string(op_long, sizeof(op_long));
						break;
					case 1:
						at->write_string(op_short, sizeof(op_short));
						break;
					default:
						at->write_string(op_num, sizeof(op_num));
						break;
				}
				at->write_int(specify_op_rat);
				at->cmd_stop_read_resp();
			}
			else
				printf("rat match specific %d\n", specify_op_rat);
		}
		#endif
		#endif // #ifdef _DEVICE_QUECTEL_BG96
		//>sophie, config to specific rat if not match
#endif

		//<sophie, comfirm attached
		printf("\nsending (%s) to modem...\n", "AT+CGATT?");
		for(i=0; i<10; i++)
		{
			at->flush(); // flush existing data
			at->lock();
			at->cmd_start("AT+CGATT?");
			at->cmd_stop();
			at->resp_start("+CGATT:");
			stat = at->read_int();
			ret = at->unlock_return_error();
			if(stat == 1)
				break;
			wait(2);
		}
		//>sophie, comfirm attached

		//<sophie, comfirm cell registered
		printf("\nsending (%s) to modem...\n", "AT+CEREG?");
		for(i=0; i<10; i++)
		{
			at->flush(); // flush existing data
			at->lock();
			at->cmd_start("AT+CEREG?");
			at->cmd_stop();
			at->resp_start("+CEREG:");
			at->read_int();	// ignore n
			stat = at->read_int();
			ret = at->unlock_return_error();
			if(stat == 1)
				break;
			wait(2);
		}
		//>sophie, comfirm cell registered

		//<no necessary
		/*
		#ifdef _DEVICE_QUECTEL_BG96
		CELLULAREXT_DEBUG("sending (%s) to modem...", cmd_attachpdp);
		at->cmd_start(cmd_attachpdp);
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		CELLULAREXT_DEBUG("result: %d", at->unlock_return_error());
		#endif
		*/
		//>no necessary
	
		//ThisThread::sleep_for(1000);
	
		device->release_at_handler(at);
	}
	else
		CELLULAREXT_ERR("Attach: get at handler fail");
}

void CellularExt_Detach(void)
{
	if(CellularExt.module == CELLMODULE_MAX)
	{
		CELLULAREXT_ERR("Detach: not cellular in list, exit");
		return;
	}
	/*
	const char cmd_bc95g[] = "AT+CGATT=0";
	const char cmd_bg96[] = "AT+QIDEACT=1";
	const char *cmd;

	//if(CellularExt.module == CELLMODULE_BG96)
	//	cmd = cmd_bg96;
	//else
		cmd = cmd_bc95g;
	*/
	const char cmd_detachcell[] = "AT+CGATT=0";
	const char cmd_detachPDP[] = "AT+QIDEACT=1";
	
	// take the ATHandler into use for sending ad hoc commands to the modem
	CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_ERR("Detach: get cellular device fail");
		return;
	}
	ATHandler *at = device->get_at_handler();
	
	if (at) {
		at->flush(); // flush existing data
		//at->set_debug(1);
		at->lock();

		//<no necessary
		/*
		#ifdef _DEVICE_QUECTEL_BG96
		CELLULAREXT_DEBUG("sending (%s & %s) to modem...", cmd_detachPDP, cmd_detachcell);
		at->cmd_start(cmd_detachPDP);
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		CELLULAREXT_DEBUG("result: %d", at->unlock_return_error());
		#endif
		*/
		//>no necessary
		
		CELLULAREXT_DEBUG("sending (%s) to modem...", cmd_detachcell);
		at->cmd_start(cmd_detachcell);
		at->cmd_stop();
		at->resp_start();
		at->resp_stop();
		CELLULAREXT_DEBUG("result: %d", at->unlock_return_error());
	
		//ThisThread::sleep_for(1000);
	
		device->release_at_handler(at);
	}
	else
		CELLULAREXT_ERR("Detach: get at handler fail");
}

//------------------------------------//
//<sophie, at handle test




void run_some_modem_command()
{

    char rsp_buf[300]="";
    int rsp_buf_len = 300;

	CELLULAREXT_DEBUG("-----------------------------------------------------------------------------");
	CELLULAREXT_DEBUG("run_some_modem_command");

    // take the ATHandler into use for sending ad hoc commands to the modem
    CellularDevice *device = CellularDevice::get_default_instance();
	if (!device)
	{
		CELLULAREXT_DEBUG("get cellular device fail");
		return;
	}

    ATHandler *at = device->get_at_handler();

    if (at) {
		at->flush(); // flush existing data
		//at->set_debug(1);

		/*
        //query modem for manufacture info
        memset(rsp_buf,0,rsp_buf_len);
        get_at_info("AT+CGMI", rsp_buf, rsp_buf_len, at);
        CELLULAREXT_DEBUG("manufacture = %s", rsp_buf); 
		//ThisThread::sleep_for(1000);

		// query modme revision
        memset(rsp_buf,0,rsp_buf_len);
        get_at_info("AT+CGMR", rsp_buf, rsp_buf_len, at);
        CELLULAREXT_DEBUG("revision = %s", rsp_buf);
		//ThisThread::sleep_for(1000);
		*/
		
		//query modem for manufacture info
        memset(rsp_buf,0,rsp_buf_len);
        get_at_info("AT+GSN", rsp_buf, rsp_buf_len, at);
        CELLULAREXT_DEBUG("imei = %s", rsp_buf); 
		//ThisThread::sleep_for(1000);

		device->release_at_handler(at);
    }
	else
		CELLULAREXT_DEBUG("get at handler fail");
	CELLULAREXT_DEBUG("-----------------------------------------------------------------------------");
    // Done with ATHandler testing

}
//>sophie, at handle test
//------------------------------------//

