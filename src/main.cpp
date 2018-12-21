#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

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
#define authorName "thirdmadman"
#define version 0.1
#define isDev false
#define powerLimit 250
#define unsafeMode true
#define minCoilResistance 0.01
#define maxCoilResistance 10
#define spalshScreen true
#define spalshScreenDuration 2000

bool wasSplash = false;
unsigned long startMillis = 0;
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void getSpalshScreen(void) {

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB10_te);
    u8g2.setCursor(25,14);
    u8g2.print("Grave mod");

    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(44,28);
    u8g2.print("made by");

    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(14,42);
    u8g2.print(authorName);

    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(36,54);
    u8g2.print("version");
    u8g2.setCursor(80,54);
    u8g2.print(version);
  } while (u8g2.nextPage());
}

void drawBattery(float voltage) {

}

void drawMainFrame(void) {
  u8g2.firstPage();
  do {
    u8g2.drawFrame(0,0,128,64);
  } while (u8g2.nextPage());
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
  if (isDev == true) {
    Serial.begin(9600); // Use fore debug?
  }
  u8g2.begin();
  startMillis=millis();
}

//<Interrup>
SIGNAL(TIMER0_COMPA_vect)
{

}
//</Interrup>



//<Loop>
void loop()
{


  if ((wasSplash == false) && (spalshScreen == true) &&  (millis()-startMillis<=spalshScreenDuration)) {
    getSpalshScreen();
  }
  else if ((wasSplash == false) && (spalshScreen == true) &&  (millis()-startMillis>spalshScreenDuration)) {
    wasSplash = true;
  }
  else {
    drawMainFrame();
  }


}
//</Loop>
