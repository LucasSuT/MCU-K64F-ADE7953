#include "mbed.h"
#include "ADE7953.h"
#include "PinNames.h"
#include <fstream>
#if 0
int main()
{
    printf("Lucas: start main\n");
    ADE7953 test(PA_9, PA_10); //correct
    printf("initial %d\n", test.init());
    // ADE7953 test2(PE_1, PE_0);
    // printf("initial %d\n", test2.init());

    double SUMA=0,SUMB=0,SUMC=0;
    int counter=0;
#if 1
    while(1)
    {
        SUMA += test.getIrmsA();
        SUMB += test.getIrmsB();
        SUMC += test.getVrms();
        counter++;
        if(counter==5)
        {
            printf("\nSUMA= %lf   SUMB= %lf   SUMC= %lf\n\n",SUMA/5,SUMB/5,SUMC/5);
            SUMA=0;
            SUMB=0;
            SUMC=0;
            counter=0;
        }

        // test.read(0x201);
        // test.verification();
        // test.read(0x301);
        // test.verification();
        // test.read(0x22D); //bit 11表示時間內電流通道沒過0 bit 12表示電流通道過0 bit 14表示時間內電壓通道沒過0 bit 15表示電壓通道過0
        // test.verification();
        // test.read(0x22E); //bit 11表示時間內電流通道沒過0 bit 12表示電流通道過0 bit 14表示時間內電壓通道沒過0 bit 15表示電壓通道過0
        // test.verification();

        printf("====================UART1=============================\n");
        printf("%-15s 0x218 = %x\n", "InstVoltage", test.getInstVoltage()); //signed
        // test.verification();
        printf("%-15s 0x21C = %lf\n", "Vrms", test.getVrms()); //unsigned
        // test.verification();
        printf("----------------Channel A------------------------\n");
        printf("%-15s 0x216 = %x\n", "InstCurrentA", test.getInstCurrentA()); //signed
        // test.verification();
        printf("%-15s 0x21A = %lf\n", "IrmsA", test.getIrmsA()); //unsigned
        // test.verification();
        // printf("%-15s 0x21E = %x\n", "ActiveEnergyA", test.getActiveEnergyA()); //signed
        printf("%-15s 0x212 = %lf\n", "InstActivePowerA", test.getInstActivePowerA());
        // // test.verification();
        // printf("%-15s 0x220 = %x\n", "ReactiveEnergyA", test.getReactiveEnergyA()); //signed
        // printf("%-15s 0x214 = %x\n", "InstReactivePowerA", test.getInstReactivePowerA());
        // // test.verification();
        // printf("%-15s 0x222 = %x\n", "ApparentEnergyA", test.getApparentEnergyA()); //signed
        // printf("%-15s 0x210 = %x\n", "InstApparentPowerA", test.getInstApparentPowerA());
        // // test.verification();

        printf("-----------------Channel B-----------------------\n");
        printf("%-15s 0x217 = %x\n", "InstCurrentB", test.getInstCurrentB());
        printf("%-15s 0x21B = %lf\n", "IrmsB", test.getIrmsB());
        // printf("%-15s 0x21F = %x\n", "ActiveEnergyB", test.getActiveEnergyB()); //有功電能
        printf("%-15s 0x213 = %lf\n", "InstActivePowerB", test.getInstActivePowerB());
        // printf("%-15s 0x221 = %x\n", "ReactiveEnergyB", test.getReactiveEnergyB()); //無功電能
        // printf("%-15s 0x215 = %x\n", "InstReactivePowerB", test.getInstReactivePowerB());
        // printf("%-15s 0x223 = %x\n", "ApparentEnergyB", test.getApparentEnergyB()); //視在電能
        // printf("%-15s 0x211 = %x\n", "InstApparentPowerB", test.getInstApparentPowerB());
        printf("=================================================\n\n");
        wait(2);

        // printf("=====================UART8============================\n");
        // printf("%-15s 0x218 = %x\n", "InstVoltage", test2.getInstVoltage());
        // printf("%-15s 0x21C = %x\n\n", "Vrms", test2.getVrms());
        // printf("----------------Channel A------------------------\n");
        // printf("%-15s 0x216 = %x\n", "InstCurrentA", test2.getInstCurrentA());
        // printf("%-15s 0x21A = %x\n", "IrmsA", test2.getIrmsA());
        // printf("%-15s 0x21E = %x\n", "ActiveEnergyA", test2.getActiveEnergyA());
        // printf("%-15s 0x220 = %x\n", "ReactiveEnergyA", test2.getReactiveEnergyA());
        // printf("%-15s 0x222 = %x\n\n", "ApparentEnergyA", test2.getApparentEnergyA());
        // printf("-----------------Channel B-----------------------\n");
        // printf("%-15s 0x217 = %x\n", "InstCurrentB", test2.getInstCurrentB());
        // printf("%-15s 0x21B = %x\n", "IrmsB", test2.getIrmsB());
        // printf("%-15s 0x21F = %x\n", "ActiveEnergyB", test2.getActiveEnergyB());
        // printf("%-15s 0x221 = %x\n", "ReactiveEnergyB", test2.getReactiveEnergyB());
        // printf("%-15s 0x223 = %x\n\n", "ApparentEnergyB", test2.getApparentEnergyB());
        // printf("=================================================\n\n");
        // wait(2);
    }
#endif
    return 0;
}
#endif