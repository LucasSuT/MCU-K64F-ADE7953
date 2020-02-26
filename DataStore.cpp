#include "mbed.h"
#include "common_def.h"

#ifdef _NO_CONFIGDATA_STORAGE
void DataStore_Init(const char *dirname){;}
int DataStore_WriteFile(const char *filename, uint8_t *buff, int buffsize, bool append){return -1;}
int DataStore_ReadFile(const char *filename, int offset, uint8_t *buff, int buffsize, int *remain){return -1;}
int DataStore_RemoveFile(const char *filename){return -1;}


#else //#ifdef _NO_CONFIGDATA_STORAGE
#ifdef _SD_CARD
#include "SDBlockDevice.h"
#include "FATFileSystem.h"
#else
#include "SPIFBlockDevice.h"
#include "LittleFileSystem.h"
#endif

#include "DataStore.h"

#include "common_debug.h"
#ifdef _DEBUG_DATASTORE
#define TAG				"DATASTORE"
#define DATASTORE_DEBUG	DEBUGMSG
#define DATASTORE_DUMP	DEBUGMSG_DUMP
#else
#define DATASTORE_DEBUG
#define DATASTORE_DUMP
#endif //_DEBUG_DATASTORE
#define DATASTORE_ERR	DEBUGMSG_ERR

#ifdef _SD_CARD
SDBlockDevice bd(MBED_CONF_SD_SPI_MOSI, MBED_CONF_SD_SPI_MISO, MBED_CONF_SD_SPI_CLK, MBED_CONF_SD_SPI_CS);
FATFileSystem fs("fs");
#else
SPIFBlockDevice bd(MBED_CONF_SPIF_DRIVER_SPI_MOSI, MBED_CONF_SPIF_DRIVER_SPI_MISO, MBED_CONF_SPIF_DRIVER_SPI_CLK, MBED_CONF_SPIF_DRIVER_SPI_CS);
LittleFileSystem fs("fs");
#endif

static bool bd_init = false;

void DataStore_Init(const char *dirname)
{
	int ret;

	//DATASTORE_DEBUG("prefert MOSI:%d, MISO:%d, SCK:%d, CS:%d", PC_12, PC_11, PC_10, PA_4);
	//DATASTORE_DEBUG("real MOSI:%d, MISO:%d, SCK:%d, CS:%d", MBED_CONF_SPIF_DRIVER_SPI_MOSI, MBED_CONF_SPIF_DRIVER_SPI_MISO, MBED_CONF_SPIF_DRIVER_SPI_CLK, MBED_CONF_SPIF_DRIVER_SPI_CS);

	if(!bd_init)
	{
		ret = bd.init();
		if(ret != 0)
		{
			DATASTORE_ERR("bd.init fail");
			goto _EXIT;
		}
		bd_init = true;
	}

	DATASTORE_DEBUG("==================================================");
	DATASTORE_DEBUG("bd size        : %lluMB", bd.size()/1024/1024);
	DATASTORE_DEBUG("bd read size   : %llu", bd.get_read_size());
	DATASTORE_DEBUG("bd program size: %llu", bd.get_program_size());
	DATASTORE_DEBUG("bd erase size  : %llu", bd.get_erase_size());
	DATASTORE_DEBUG("==================================================\n");

	/*
	ret = fs.format(&bd);
	if(ret != 0)
	{
		DATASTORE_ERR("fs.format fail");
		goto _DEINIT;
	}
	*/
	
	ret = fs.mount(&bd);
	if(ret != 0)
	{
		DATASTORE_ERR("fs.mount fail");
		//goto _DEINIT;
		
		ret = fs.format(&bd);
		if(ret != 0)
		{
			DATASTORE_ERR("fs.format fail");
			goto _DEINIT;
		}
		
		ret = fs.mount(&bd);
		if(ret != 0)
		{
			DATASTORE_ERR("fs.mount fail");
			goto _DEINIT;
		}
	}

	ret = fs.mkdir(dirname, 0777);
	if(ret != 0)
	{
		DATASTORE_ERR("fs.mkdir fail");
	}

	fs.unmount();
_DEINIT:
    //bd.deinit();
_EXIT:
	DATASTORE_DEBUG("DataStore_Init end");
}

int DataStore_WriteFile(const char *filename, uint8_t *buff, int buffsize, bool append)
{
	int ret;
	File file;

	if(!bd_init)
	{
		ret = bd.init();
		if(ret != 0)
		{
			DATASTORE_ERR("bd.init fail");
			goto _EXIT;
		}
		bd_init = true;
	}
	ret = fs.mount(&bd);
	if(ret != 0)
	{
		DATASTORE_ERR("fs.mount fail");
		goto _DEINIT;
	}

	if(append)
		ret = file.open(&fs, filename, O_CREAT | O_WRONLY | O_BINARY | O_APPEND);
	else
		ret = file.open(&fs, filename, O_CREAT | O_WRONLY | O_BINARY);
	if(ret != 0)
	{
		DATASTORE_ERR("fs.open fail");
		goto _UNMOUNT;
	}
	ret = file.write(buff, buffsize);
	file.close();

_UNMOUNT:
	fs.unmount();
_DEINIT:
	//bd.deinit();
_EXIT:
	DATASTORE_DEBUG("DataStore_WriteFile \"%s\" end %d", filename, ret);
	return ret;
}

int DataStore_ReadFile(const char *filename, int offset, uint8_t *buff, int buffsize, int *remain)
{
	int ret;
	int fsize;
	int readsize;
	File file;

	*remain = 0;
	memset(buff, 0, buffsize);

	if(!bd_init)
	{
		ret = bd.init();
		if(ret != 0)
		{
			DATASTORE_ERR("bd.init fail");
			goto _EXIT;
		}
		bd_init = true;
	}
	ret = fs.mount(&bd);
	if(ret != 0)
	{
		DATASTORE_ERR("fs.mount fail");
		goto _DEINIT;
	}

	ret = file.open(&fs, filename, O_RDONLY | O_BINARY);
	if(ret != 0)
	{
		DATASTORE_ERR("fs.open fail");
		goto _UNMOUNT;
	}
	fsize = file.size();

	if(!buff || !buffsize)
	{
		ret = 0;
		DATASTORE_ERR("buffer[%d] 0x%x error", buffsize, buff);
		goto _FCLOSE;
	}

	readsize = ((fsize-offset) < buffsize)? (fsize-offset) : buffsize;
	file.seek(offset, SEEK_SET);
	ret = file.read(buff, readsize);
	if(ret > 0)
		*remain = fsize - offset - ret;

_FCLOSE:
	file.close();	
_UNMOUNT:
	fs.unmount();
_DEINIT:
	//bd.deinit();
_EXIT:
	DATASTORE_DEBUG("DataStore_ReadFile \"%s\" end %d", filename, ret);
	return ret;
}

int DataStore_RemoveFile(const char *filename)
{
	int ret;

	if(!bd_init)
	{
		ret = bd.init();
		if(ret != 0)
		{
			DATASTORE_ERR("bd.init fail");
			goto _EXIT;
		}
		bd_init = true;
	}
	ret = fs.mount(&bd);
	if(ret != 0)
	{
		DATASTORE_ERR("fs.mount fail");
		goto _DEINIT;
	}

	ret = fs.remove(filename);
	if(ret != 0)
	{
		DATASTORE_ERR("fs.remove fail");
	}

_UNMOUNT:
	fs.unmount();
_DEINIT:
	//bd.deinit();
_EXIT:
	DATASTORE_DEBUG("DataStore_DeleteFile \"%s\" end %d", filename, ret);
	return ret;
}

#endif //#else //#ifdef _NO_CONFIGDATA_STORAGE

