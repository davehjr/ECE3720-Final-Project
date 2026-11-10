
#include <hidef.h>
#include "derivative.h"

//lighting, 00SG/SY_SR/MG/MY/MR 
#define MAIN_GREEN_SIDE_RED    0x0C //0000_1100
#define MAIN_YELLOW_SIDE_RED   0x0A //0000_1010
#define ALL_RED                0x09 //0000_1001
#define MAIN_RED_SIDE_GREEN    0x21 //0010_0001
#define MAIN_RED_SIDE_YELLOW   0x11 //0001_0001

// states
#define S_MAIN_GREEN   0
#define S_MAIN_YELLOW  1
#define S_RED_A        2    // all red, leads to side green
#define S_SIDE_GREEN   3
#define S_SIDE_YELLOW  4
#define S_RED_B        5    // all red, leads to main green
#define S_EMERGENCY    6

// timing
#define GREEN_MS        15000 //fixed mode green for when if pi connection lost
#define SIDE_MIN_MS      5000
#define SIDE_MAX_MS     15000
#define YELLOW_MS        3000
#define ALL_RED_MS       1000

// placeholder inputs
// later these will come from the Pi: Pi_Alive(), Pi_Cars(), Pi_WaitSec()
unsigned char piAlive = 0;      // 0 = no Pi, run fixed timing
unsigned char sideCars = 0;     // 0-3 cars waiting on the side street
unsigned char sideWait = 0;     // 0-63 seconds the first car has waited
unsigned char emergency = 0;    // 1 = emergency vehicle

unsigned char state = S_RED_B;  // start safe, leads to main green
unsigned int elapsed = 0;       // ms spent in the current state

void PLL_Init(void);
void Timer_Init(void);
void IO_Init(void);
void delay_ms(unsigned int ms);
void Go(unsigned char newState, unsigned char pattern);
unsigned char SideNeedsGreen(void);

void main(void)
{
    PLL_Init();
    Timer_Init();
    IO_Init();

    Go(S_RED_B, ALL_RED); //start at all red

    for(;;)
    {
        delay_ms(1);        // one millisecond passes each loop
        elapsed++;

        // emergency overrides everything
        if(emergency)
        {
            if(state != S_EMERGENCY)  //emergency detected but not there yet
            {
                Go(S_EMERGENCY, ALL_RED); //set emergency
            }
        }
        else if(state == S_EMERGENCY) //still at emergency when its over
        {
            Go(S_RED_B, ALL_RED);       // emergency over: all red, then main green
        }
        else if(state == S_MAIN_GREEN)
        {
            if(SideNeedsGreen() || (!piAlive && elapsed >= GREEN_MS)) //at green but cars waited long enough OR lost comms to Pi
            {
                Go(S_MAIN_YELLOW, MAIN_YELLOW_SIDE_RED);
            }
        }
        else if(state == S_MAIN_YELLOW)
        {
            if(elapsed >= YELLOW_MS) Go(S_RED_A, ALL_RED);
        }
        else if(state == S_RED_A)
        {
            if(elapsed >= ALL_RED_MS) Go(S_SIDE_GREEN, MAIN_RED_SIDE_GREEN);
        }
        else if(state == S_SIDE_GREEN)
        {
            if(elapsed >= SIDE_MAX_MS ||
               (elapsed >= SIDE_MIN_MS && piAlive && sideCars == 0) ||
               (!piAlive && elapsed >= GREEN_MS)) //max side green OR pi alive and 5s passed and no cars waiting OR pi is dead and 15s passed
            {
                Go(S_SIDE_YELLOW, MAIN_RED_SIDE_YELLOW);
            }
        }
        else if(state == S_SIDE_YELLOW)
        {
            if(elapsed >= YELLOW_MS) Go(S_RED_B, ALL_RED);
        }
        else if(state == S_RED_B)
        {
            if(elapsed >= ALL_RED_MS) Go(S_MAIN_GREEN, MAIN_GREEN_SIDE_RED);
        }
    }
}

// side street has waited long enough for the number of cars there
unsigned char SideNeedsGreen(void)
{
    if(!piAlive) return 0; //is our pi comms working?
    if(sideCars == 1 && sideWait >= 45) return 1;
    if(sideCars == 2 && sideWait >= 30) return 1;
    if(sideCars == 3 && sideWait >= 15) return 1;
    return 0;
}

// switch to a new state, restart the state timer, set the lights
void Go(unsigned char newState, unsigned char pattern)
{
    state = newState;
    elapsed = 0;
    PORTB = pattern;
}

void PLL_Init(void)     // from lab 5: 8 MHz -> 24 MHz bus
{
    CLKSEL &= 0x7F;
    PLLCTL |= 0x40;
    SYNR = 0x02;
    REFDV = 0x00;
    while(!(CRGFLG & 0x08)) {}
    CLKSEL |= 0x80;
}

void Timer_Init(void)   // from Reaction Time lab
{
    TSCR1 = 0x80;       // enable timer
    TSCR2 = 0x07;       // prescaler 128 -> 187,500 ticks/sec
    TIOS = 0x40;        // channel 6 = output compare
}

void IO_Init(void)
{
    DDRJ |= 0x02;       // PJ1 output
    PTJ &= 0xFD;        // PJ1 low = enable the PORTB LEDs
    DDRB = 0xFF;        // PB7-PB0 outputs
    PORTB = ALL_RED;
}

void delay_ms(unsigned int ms)   // copy and pasted over from lab 5
{
    unsigned int i;

    for(i = 0; i < ms; i++)
    {
        TFLG1 = 0x40;
        TC6 = TCNT + 188;       // ~1 ms
        while(!(TFLG1 & 0x40)) {}
    }
}
