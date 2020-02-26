#include <mbed.h>
#include <stdint.h>

typedef int REG_VALUE;
typedef enum Status
{
    Fail = 0,
    Success = 1,
} Status;

class ADE7953
{
public:
    bool init();
    int getInstVoltage();
    int getVrms();
    int getInstCurrentA();
    int getIrmsA();
    int getActiveEnergyA();
    int getReactiveEnergyA();
    int getApparentEnergyA();
    int getInstCurrentB();
    int getIrmsB();
    int getActiveEnergyB();
    int getReactiveEnergyB();
    int getApparentEnergyB();
    ADE7953()
    {
        uart=new UARTSerial(PA_9, PA_10, 4800);
    }
    ADE7953(PinName tx,PinName rx)
    {
        uart = new UARTSerial(tx, rx, 4800);
    }
    int* verification();

private :
    int getRegisterSize_Byte(uint16_t);
    bool write(uint16_t, uint32_t);
    REG_VALUE read(uint16_t);
    const uint8_t ADE_READ = 0x35;
    const uint8_t ADE_WRITE = 0xCA;
    UARTSerial *uart=nullptr;
};