#include "ADE7953.h"

bool ADE7953::init()
{
    if(read(0x102) != 0x8004)return Fail; //check UART communication
    write(0x102,0x04); //lock the communication
    write(0x280, 0x489500); //Current channel A gain calibration
    write(0x28C, 0x499000); //Current channel B gain calibration
    write(0x281, 0x188500); //Voltage channel gain calibration
    write(0x282, 0x699500); //Current channel A Active power gain calibration
    write(0x28E, 0x295950); //Current channel B Active power gain calibration
    write(0x040, 0x07); //internal registers write protection
    read(0x800); //基準電壓輸入
    return Success;
}

int ADE7953::getInstVoltage() //Instantaneous voltage (voltage channel)
{
    return (read(0x218));
}

double ADE7953::getVrms() //VRMS register
{
    return (read(0x21C)/10000.0);
}

//*****************************Channel A*****************************

int ADE7953::getInstCurrentA() //Instantaneous current (Current Channel A)
{
    return (read(0x216));
}

double ADE7953::getIrmsA() //IRMS register (Current Channel A)
{
    return (read(0x21A)/100000.0);
}

int ADE7953::getActiveEnergyA() //Active energy (Current Channel A)
{
    return (read(0x21E));
}

double ADE7953::getInstActivePowerA() //Active power (Current Channel A)
{
    return read(0x212)/100.0;
}

int ADE7953::getReactiveEnergyA() //Reactive energy (Current Channel A)
{
    return (read(0x220));
}

int ADE7953::getInstReactivePowerA()
{
    return read(0x214);
}

int ADE7953::getApparentEnergyA() //Apparent energy (Current Channel A)
{
    return (read(0x222));
}

int ADE7953::getInstApparentPowerA()
{
    return read(0x210);
}

int ADE7953::getPowerFactorA()
{
    return read(0x10A);
}

//*****************************Channel B*****************************

int ADE7953::getInstCurrentB() //Instantaneous current (Current Channel B)
{
    return (read(0x217));
}

double ADE7953::getIrmsB() //IRMS register (Current Channel B)
{
    return (read(0x21B)/100000.0);
}

int ADE7953::getActiveEnergyB() //Active energy (Current Channel B)
{
    return (read(0x21F));
}

double ADE7953::getInstActivePowerB() //Active power (Current Channel B)
{
    return read(0x213)/100.0;
}

int ADE7953::getReactiveEnergyB() //Reactive energy (Current Channel B)
{
    return (read(0x221));
}

int ADE7953::getInstReactivePowerB()
{
    return read(0x215);
}

int ADE7953::getApparentEnergyB() //Apparent energy (Current Channel B)
{
    return (read(0x223));
}

int ADE7953::getInstApparentPowerB()
{
    return read(0x211);
}

int ADE7953::getPowerFactorB()
{
    return read(0x10B);
}

//*****************************Test function*****************************

int* ADE7953::verification()
{
    int *a = new int[3];
    a[0] = read(0x0FD); //LAST_OP
    a[1] = read(0x1FE); //LAST_ADD
    a[2] = read(0x3FF); //LAST_RWDATA
    printf("OP= 0x%x ADDR= 0x%x DATA= 0x%x\n", *a, *(a + 1), *(a + 2));
    return a;
}

bool ADE7953::write(uint16_t Reg, uint32_t val)
{
    uint8_t txb[7] = {ADE_WRITE, (uint8_t)(Reg >> 8), (uint8_t)Reg, (uint8_t)val, 
                        (uint8_t)(val >> 8), (uint8_t)(val >> 16), (uint8_t)(val >> 24)};
    int registerSize = getRegisterSize_Byte(Reg);
    if (registerSize < 5)
    {
        if (uart->writable())
        {
            uart->write(txb, registerSize+3);
            wait_ms(100);
            return Success;
        }
        // else printf("Write - cant write!\n");
    }
    return Fail;
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
            if (uart->writable() && !lock)
            {
                    uart->write(txb, sizeof(txb));
                    lock=1;
            }
            // else printf("Read - cant write!\n");
            if (uart->readable())
            {
                uart->read(rxb, registerSize);
                lock=0;
                return *regValue;
            }
            // else printf("Read - cant read!\n");
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
