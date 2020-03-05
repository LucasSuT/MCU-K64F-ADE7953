#include "mbed.h"
#include "ADE7953.h"
#include "PinNames.h"
#include <fstream>
#if 1
int main()
{
    printf("Lucas: start main\n");
    ADE7953 sample(PA_9, PA_10);
    printf("initial %d\n", sample.init());
    while (1)
    {
        printf("====================UART1=============================\n");
        printf("%-15s 0x21C = %lf\n", "Vrms", sample.getVrms()); //unsigned
        printf("----------------Channel A------------------------\n");
        printf("%-15s 0x21A = %lf\n", "IrmsA", sample.getIrmsA()); //unsigned
        printf("%-15s 0x212 = %lf\n", "InstActivePowerA", sample.getInstActivePowerA());
        printf("-----------------Channel B-----------------------\n");
        printf("%-15s 0x21B = %lf\n", "IrmsB", sample.getIrmsB());
        printf("%-15s 0x213 = %lf\n", "InstActivePowerB", sample.getInstActivePowerB());
        printf("=================================================\n\n");
        wait(1);
    }
    return 0;
}
#endif