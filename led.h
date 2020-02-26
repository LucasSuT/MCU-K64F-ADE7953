#define LED_ON						1
#define LED_OFF						0

#define LED_BLINK_FAST_MS			100
#define LED_BLINK_NORMAL_MS			1000
#define LED_BLINK_SLOW_MS			3000


void Led_RedOnOff(bool onoff);
void Led_GreenOnOff(bool onoff);
void Led_RedBlink_Ms(int ms);
void Led_GreenBlink_Ms(int ms);

// led green - on / off
#define LED_RED_ON()						Led_RedOnOff(LED_ON)
#define LED_RED_OFF()						Led_RedOnOff(LED_OFF)

// led green - on / off
#define LED_GREEN_ON()						Led_GreenOnOff(LED_ON)
#define LED_GREEN_OFF()						Led_GreenOnOff(LED_OFF)

// led red - blink
#define LED_RED_BLINK_FAST()				Led_RedBlink_Ms(LED_BLINK_FAST_MS)
#define LED_RED_BLINK_NORMAL()				Led_RedBlink_Ms(LED_BLINK_NORMAL_MS)
#define LED_RED_BLINK_SLOW()				Led_RedBlink_Ms(LED_BLINK_SLOW_MS)

// led green - blink
#define LED_GREEN_BLINK_FAST()				Led_GreenBlink_Ms(LED_BLINK_FAST_MS)
#define LED_GREEN_BLINK_NORMAL()			Led_GreenBlink_Ms(LED_BLINK_NORMAL_MS)
#define LED_GREEN_BLINK_SLOW()				Led_GreenBlink_Ms(LED_BLINK_SLOW_MS)

