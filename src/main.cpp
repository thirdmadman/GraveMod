#include <Arduino.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void drawMainFrame(void) {

}

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
  u8g2.firstPage();
  do {
    u8g2.drawFrame(0,0,128,64);
  } while (u8g2.nextPage());

}
//</Loop>
