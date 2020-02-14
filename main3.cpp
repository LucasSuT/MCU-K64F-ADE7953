#include "mbed.h"
#include "ADE7953.h"

int main()
{
    printf("Lucas: start main\n");
    // uint8_t txb[] = { 0x35,0x02,0x03 }; 
    // uint8_t rxb[4]={0};
    // const uint32_t *reg203 = (const uint32_t *)rxb;
    // UARTSerial uart1(PA_9, PA_10, 4800);
    // while (1)
    // {
    //     if (uart1.readable())
    //         printf("Lucas: unready to read\n");
    //     uart1.write((void*)txb,sizeof(txb));
    //     if (uart1.readable())
    //         uart1.read((void *)rxb, (size_t)3);
    //     printf("Lucas: 0x%x\n",*reg203);
    //     ThisThread::sleep_for(500);
    // }
    ADE7953 test;
    printf("initial %d\n",test.init());
    printf("in main 0x%x\n", test.read(0x102));
    printf("in main 0x%x\n", test.read(0x103));
    printf("in main 0x%x\n", test.read(0x203));
    printf("in main 0x%x\n", test.read(0x102));
    printf("in main write %d\n", test.write(0x102,0x00));
    printf("in main 0x%x\n", test.read(0x102));
    printf("Lucas: end main\n"); //Lu
    return 0;
}