#include "ADE7953.h"

UARTSerial uart(PA_9, PA_10, 4800);

bool ADE7953::init()
{
    return (read(0x203) == 0xE419) ? SUCCESS : FAIL;
}

bool ADE7953::write(uint16_t Reg, uint32_t val)
{
    uint8_t txb[7] = {ADE_WRITE, (uint8_t)(Reg >> 8), (uint8_t)Reg, (uint8_t)val, 
                        (uint8_t)(val >> 8), (uint8_t)(val >> 16), (uint8_t)(val >> 24)};
    int registerSize = getRegisterSize_Byte(Reg);
    if (registerSize < 5)
    {
        if (uart.writable())
        {
            uart.write(txb, registerSize+3);
            wait_ms(100);
            return SUCCESS;
        }
        else printf("Write - cant write!\n");
    }
    return FAIL;
}

REG_VALUE ADE7953::read(uint16_t Reg)
{
    uint8_t txb[3] = {ADE_READ, (uint8_t)(Reg >> 8), (uint8_t)Reg};
    uint8_t rxb[4] = {0};
    uint32_t *regValue = (uint32_t *)rxb;
    int lock=0;
    int registerSize = getRegisterSize_Byte(Reg);
    if (registerSize < 5)
    { 
        while (1)
        {
            if (uart.writable() && !lock)
            {
                    uart.write(txb, sizeof(txb));
                    lock=1;
            }
            else printf("Read - cant write!\n");
            if (uart.readable())
            {
                uart.read(rxb, registerSize);
                lock=0;
                return *regValue;
            }
            else printf("Read - cant read!\n");
            wait_ms(100);
        }
    }
    return 0;
}

int ADE7953::getRegisterSize_Byte(uint16_t Reg)
{
    int count = 0;

    if (Reg < 0x100 || (Reg == 0x702) || (Reg == 0x800))
    {
        count = 1;
    } //8Bit inkl. 0x702/0x800
    else if (Reg < 0x200)
    {
        count = 2;
    } //16Bit
    else if (Reg < 0x300)
    {
        count = 3;
    } //24Bit
    else if (Reg < 0x400)
    {
        count = 4;
    } //32Bit
    else
    {
        count = 5;
    } //soft register
    return count;
}
