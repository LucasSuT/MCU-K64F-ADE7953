#include "mbed.h"
#include "ADE7953.h"
#include "PinNames.h"
#if 1
int main()
{
    printf("Lucas: start main\n");
    ADE7953 test(PA_9,PA_10);
    ADE7953 test2(PE_1,PE_0);
    printf("initial %d\n", test.init());
    printf("initial %d\n", test2.init());
    // printf("in main 0x%x\n", test.read(0x102));
    // printf("in main 0x%x\n", test.read(0x103));
    // printf("in main 0x%x\n", test.read(0x203));
    // printf("in main 0x%x\n", test.read(0x102));
    // printf("in main write %d\n", test.write(0x102, 0x8004));
    // printf("in main 0x%x\n", test.read(0x102));
#if 1
    while(1)
    {
        // int *a=test.verification();
        // printf("OP= 0x%x ADDR= 0x%x DATA= 0x%x\n",a,a+1,a+2);
        printf("====================UART1=============================\n");
        printf("%-15s 0x218 = %x\n", "InstVoltage", test.getInstVoltage()); //signed
        // test.verification();
        printf("%-15s 0x21C = %x\n", "Vrms", test.getVrms()); //unsigned
        // test.verification();
        printf("----------------Channel A------------------------\n");
        printf("%-15s 0x216 = %x\n", "InstCurrentA", test.getInstCurrentA()); //signed
        // test.verification();
        printf("%-15s 0x21A = %x\n", "IrmsA", test.getIrmsA()); //unsigned
        // test.verification();
        printf("%-15s 0x21E = %x\n", "ActiveEnergyA", test.getActiveEnergyA()); //signed
        // test.verification();
        printf("%-15s 0x220 = %x\n", "ReactiveEnergyA", test.getReactiveEnergyA()); //signed
        // test.verification();
        printf("%-15s 0x222 = %x\n", "ApparentEnergyA", test.getApparentEnergyA()); //signed
        // test.verification();
        printf("-----------------Channel B-----------------------\n");
        printf("%-15s 0x217 = %x\n", "InstCurrentB", test.getInstCurrentB());
        printf("%-15s 0x21B = %x\n", "IrmsB", test.getIrmsB());
        printf("%-15s 0x21F = %x\n", "ActiveEnergyB", test.getActiveEnergyB());
        printf("%-15s 0x221 = %x\n", "ReactiveEnergyB", test.getReactiveEnergyB());
        printf("%-15s 0x223 = %x\n", "ApparentEnergyB", test.getApparentEnergyB());
        printf("=================================================\n\n");
        wait(3);

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
        // wait(1);
    }
#endif
    return 0;
}
#endif