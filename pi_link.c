
#include "derivative.h"
#include "clock.h"
#include "pi_link.h"

unsigned char cars = 0;       // C1-C0
unsigned char waitSec = 0;    // T5-T0
unsigned char alive = 0;      // 1 = Pi is talking
unsigned int lastRx = 0;      // clock_ms() when the last byte arrived

void SCI1_Init(void)
{
    SCI1BDH = 0x00;
    SCI1BDL = 0x9C;    // 156 ~ 24MHz / (16 * 9600)
    SCI1CR1 = 0x00;    // 8-bit, no parity
    SCI1CR2 = 0x04;    // receiver only, this code only listens to data
}

void Pi_Read(void)
{
    unsigned char data; //our C1 C2 T5 T4 T3 T2 T1 T0, ex 0x70 = 1 car waiting for 48s 

    if(SCI1SR1 & 0x20)              // 0010_0000 -> RDRF (receive data register flag) = 1, a byte is waiting
    {
        data = SCI1DRL;             // read incoming data and clear the flag
        cars = (data >> 6) & 0x03;  // shift C1 and C0 right 6, and clear the rest of the msg to isolate C1 and C0
        waitSec = data & 0x3F;      // bottom 6 bits = T5-T0
        alive = 1;
        lastRx = clock_ms();   //note when msg arrived
    }

    // just incase pi quiet for 500 ms, ignore the camera
    if((unsigned int)(clock_ms() - lastRx) > 500)
    {
        alive = 0;
        cars = 0;
        waitSec = 0;
    }
}

unsigned char Pi_Cars(void)    { return cars; }
unsigned char Pi_WaitSec(void) { return waitSec; }
unsigned char Pi_Alive(void)   { return alive; }
