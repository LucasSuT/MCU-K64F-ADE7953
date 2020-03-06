#ifndef _DATA_STORE_H
#define _DATA_STORE_H

void DataStore_Init(const char *dirname);
int DataStore_WriteFile(const char *filename, uint8_t *buff, int buffsize, bool append);
int DataStore_ReadFile(const char *filename, int offset, uint8_t *buff, int buffsize, int *remain);
int DataStore_RemoveFile(const char *filename);

#endif
