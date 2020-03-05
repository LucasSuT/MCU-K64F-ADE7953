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
    double getVrms();

    int getInstCurrentA();
    double getIrmsA();
    int getActiveEnergyA();
    double getInstActivePowerA();
    int getReactiveEnergyA();
    int getInstReactivePowerA();
    int getApparentEnergyA();
    int getInstApparentPowerA();
    int getPowerFactorA();

    int getInstCurrentB();
    double getIrmsB();
    int getActiveEnergyB();
    double getInstActivePowerB();
    int getReactiveEnergyB();
    int getInstReactivePowerB();
    int getApparentEnergyB();
    int getInstApparentPowerB();
    int getPowerFactorB();
    ADE7953()
    {
        uart=new UARTSerial(PA_9, PA_10, 4800);
    }
    ADE7953(PinName tx,PinName rx)
    {
        uart = new UARTSerial(tx, rx, 4800);
    }
    int* verification();
    bool write(uint16_t, uint32_t);
    REG_VALUE read(uint16_t);

private :
    int getRegisterSize_Byte(uint16_t);
    
    const uint8_t ADE_READ = 0x35;
    const uint8_t ADE_WRITE = 0xCA;
    UARTSerial *uart=nullptr;
};