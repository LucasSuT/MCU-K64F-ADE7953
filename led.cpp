#include "common_def.h"
#if !defined(_PROJECT_SLC) && !defined(_PROJECT_SPM)

void Led_RedOnOff(bool onoff) {;}
void Led_GreenOnOff(bool onoff) {;}
void Led_RedBlink_Ms(int ms) {;}
void Led_GreenBlink_Ms(int ms) {;}

#else // #if !defined(_PROJECT_SLC) || !defined(_PROJECT_SPM)

#include "mbed.h"
#include "led.h"

#include "common_debug.h"
#ifdef _DEBUG_LED
#define TAG					"LED"
#define LED_DEBUG			DEBUGMSG
#define LED_DUMP			DEBUGMSG_DUMP
#define LED_ERR				DEBUGMSG_ERR
#else
#define LED_DEBUG
#define LED_DUMP
#define LED_ERR
#endif //_DEBUG_LED

//-----------------------------------------------------------------------------------//
// project hw define
//-----------------------------------------------------------------------------------//
// SLC hardware A06
// LED on/off - This could be different among boards

#define LED_RED_PIN								PD_11
#define LED_GREEN_PIN							PD_12

//-----------------------------------------------------------------------------------//
// local structure declaration and global veriable
//-----------------------------------------------------------------------------------//
typedef struct {
	bool				TimerLedRed;			// flag for launch timer
	bool				TimerLeGreen;			// flag for launch timer
} _Led;

static _Led Led = { .TimerLedRed = false,
					.TimerLeGreen = false};

//-----------------------------------------------------------------------------------//
// function declaration
//-----------------------------------------------------------------------------------//
static void Led_TimerLedRedFxn(void const *n);
static void Led_TimerLedGreenFxn(void const *n);


//-----------------------------------------------------------------------------------//
// driver
//-----------------------------------------------------------------------------------//
DigitalOut LedRed(LED_RED_PIN, LED_OFF);
DigitalOut LedGreen(LED_GREEN_PIN, LED_OFF);

//-----------------------------------------------------------------------------------//
// timer
//-----------------------------------------------------------------------------------//
RtosTimer Led_TimerLedRed(Led_TimerLedRedFxn, osTimerPeriodic, (void *)0);
RtosTimer Led_TimerLedGreen(Led_TimerLedGreenFxn, osTimerPeriodic, (void *)0);


// timer - blink led red
#define LED_STOPTIMER_LEDRED()					do { \
													if(Led.TimerLedRed) Led_TimerLedRed.stop(); \
													Led.TimerLedRed = false; \
												} while(0)
#define LED_RELAUNCHTIMER_LEDRED(period)		do { \
													if(Led.TimerLedRed) Led_TimerLedRed.stop(); \
													Led_TimerLedRed.start(period); \
													Led.TimerLedRed = true; \
												} while(0)
// timer - blink led green
#define LED_STOPTIMER_LEDGREEN()				do { \
													if(Led.TimerLeGreen) Led_TimerLedGreen.stop(); \
													Led.TimerLeGreen = false; \
												} while(0)
#define LED_RELAUNCHTIMER_LEDGREEN(period)		do { \
													if(Led.TimerLeGreen) Led_TimerLedGreen.stop(); \
													Led_TimerLedGreen.start(period); \
													Led.TimerLeGreen = true; \
												} while(0)
												

//-----------------------------------------------------------------------------------//
// timer functions
//-----------------------------------------------------------------------------------//
static void Led_TimerLedRedFxn(void const *n)
{
	LedRed = !LedRed;
}
static void Led_TimerLedGreenFxn(void const *n)
{
	LedGreen = !LedGreen;
}

//-----------------------------------------------------------------------------------//
// functions
//-----------------------------------------------------------------------------------//
void Led_RedOnOff(bool onoff)
{
	LED_STOPTIMER_LEDRED();
	if(onoff)
		LedRed = LED_ON;
	else
		LedRed = LED_OFF;
}

void Led_GreenOnOff(bool onoff)
{
	LED_STOPTIMER_LEDGREEN();
	if(onoff)
		LedGreen = LED_ON;
	else
		LedGreen = LED_OFF;
}

void Led_RedBlink_Ms(int ms)
{
	LED_RELAUNCHTIMER_LEDRED(ms);
}

void Led_GreenBlink_Ms(int ms)
{
	LED_RELAUNCHTIMER_LEDGREEN(ms);
}

#endif //#else // #ifndef _PROJECT_SLC
