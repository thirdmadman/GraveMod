#include <U8g2lib.h>

#define buttonFire 3
#define buttonUp 4
#define buttonDown 4
#define fireMosfets 9
#define measureMosfet 8
#define measureBattery A0
#define measureVoltagedivider A1
#define lowBattery 3.7
#define lowCriticalBattery 3.4
#define overchargedBattery 4.35
#define mosfetResistance 0.01
#define voltagedividerR1 3
#define authorName thirdmadman
#define version 0.1
#define isDev true
#define powerLimit 250
#define unsafeMode
#define minCoilResistance 0.01
#define maxCoilResistance 10

#if ENABLED(unsafeMode)
	#define minCoilResistance 0.0001
	#define maxCoilResistance 1000
	#define lowBattery 3.4
	#define lowCriticalBattery 3.2
#endif



U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
void setup()
{
//<Interrupts>
// that's making interrupts about once a second
	OCR0A = 0xAF;
	TIMSK0 |= _BV(OCIE0A);
//</Interrupts>

//<PWM>
//that's making PWM 10 bit (1024) on 15625 Hz
	TCCR1A = TCCR1A & 0xe0 | 3;
	TCCR1B = TCCR1B & 0xe0 | 0x09;
//</PWM>
	if (isDev == true) {
		Serial.begin(9600); // Use fore debug?
	}
  u8g2.begin();
}

//<Interrup>
SIGNAL(TIMER0_COMPA_vect)
{

}
//</Interrup>

//<Loop>
void loop()
{

}
//</Loop>
