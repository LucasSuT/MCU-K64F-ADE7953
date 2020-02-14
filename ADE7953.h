#include <mbed.h>
#include <stdint.h>

typedef int REG_VALUE;
enum Initial
{
    FAIL = 0,
    SUCCESS = 1,
};

class ADE7953
{
public:
    bool init();
    bool write(uint16_t, uint32_t);
    REG_VALUE read(uint16_t);
    int getRegisterSize_Byte(uint16_t);
    

private : 
    const uint8_t ADE_READ = 0x35;
    const uint8_t ADE_WRITE = 0xCA;
};