#include <Arduino.h>
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

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
#define maxCharchedBattery 4.25
#define mosfetResistance 0.01
#define voltagedividerR1 3
#define authorName "thirdmadman"
#define version 0.1
#define isDev true
#define powerLimit 250
#define unsafeMode true
#define minCoilResistance 0.01
#define maxCoilResistance 10
#define spalshScreen false
#define spalshScreenDuration 2000
#define lowestResistance 0.015
#define lowestResistanceUnsafe 0.0001
#define highestResistance 15
#define startPower 20
#define maxPower 300



bool wasSplashScreen = false;
float batteryVoltage = 2,
coilResistance = 0,
power;

unsigned long startMillis = 0;




String getBatteryState(float voltage) {
  if (voltage>maxCharchedBattery) {
    //don't let user use this battery
    return "BATTERY_OVERCHARGED";
  }
  else if ((voltage<=maxCharchedBattery) && (voltage>lowBattery)) {
    return "BATTERY_NORMAL";

  }
  else if ((voltage<=lowBattery) && (voltage>lowCriticalBattery)) {
    //notice - battery is low
    return "BATTERY_LOW";
  }
  else if ((voltage<=lowCriticalBattery) && (voltage>(lowCriticalBattery-1))) {
    //don't let user discharge battery more
    return "BATTERY_LOWCRITICAL";

  }
  else if (voltage<=(lowCriticalBattery-1)) {
    //check your battery - is it there?
    return "BATTERY_NONE";
  }

  return "";

}

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

  String state = getBatteryState(voltage);
  //Serial.println(state);

  if (state=="BATTERY_OVERCHARGED") {
    //don't let user use this battery
    u8g2.drawFrame(96,4,25,10);
    u8g2.drawBox(121, 6, 3, 6);
    u8g2.setFont(u8g2_font_ncenB08_te);
    u8g2.setCursor(104,13);
    u8g2.print("!!!");
  }
  else if (state=="BATTERY_NORMAL") {
    u8g2.drawFrame(96,4,25,10);
    u8g2.drawBox(121, 6, 3, 6);
    int batteryPix = floor((voltage - lowBattery) / ((maxCharchedBattery - lowBattery) / 25.0));
		if (batteryPix > 25) {
			batteryPix = 25;
		}
    u8g2.drawBox(97, 5, batteryPix, 8);

  }
  else if (state=="BATTERY_LOW") {
    //notice - battery is low
    u8g2.drawFrame(96,4,25,10);
    u8g2.drawBox(121, 6, 3, 6);
    u8g2.setFont(u8g2_font_ncenB08_te);
    u8g2.setCursor(72,13);
    u8g2.print(voltage);
    u8g2.setFont(u8g2_font_ncenB08_te);
    u8g2.setCursor(109,13);
    u8g2.print("!");
  }
  else if (state=="BATTERY_LOWCRITICAL") {
    //don't let user discharge battery more
    u8g2.drawFrame(96,4,25,10);
    u8g2.drawBox(121, 6, 3, 6);
    u8g2.drawLine(96,4,120,13);
    u8g2.drawLine(96,13,120,4);


  }
  else if (state == "BATTERY_NONE") {
    //check your battery - is it there?
    u8g2.setFont(u8g2_font_ncenB08_te);
    u8g2.setCursor(108,13);
    u8g2.print("?");
  }
  u8g2.setFont(u8g2_font_ncenB08_te);
  u8g2.setCursor(72,13);
  u8g2.print(voltage);


}

void drawResitance(float resistance) {

  //float setResistance = resitace + mosfetResis;
  //u8g2.print(setResistance);

  if (resistance > highestResistance) {
    u8g2.setFont(u8g2_font_ncenB08_te);
    u8g2.setCursor(8,13);
    u8g2.print("coil none");
  }
  else if (resistance >= 0.01) {
    u8g2.setFont(u8g2_font_6x12_t_symbols);
    u8g2.drawGlyph(19, 13, 0x2126);
    u8g2.setFont(u8g2_font_ncenB08_te);

    u8g2.setCursor(27,13);
    u8g2.print(resistance);
  }
  else if (resistance < 0.01 && resistance > lowestResistanceUnsafe) {
    u8g2.setFont(u8g2_font_6x12_t_symbols);
    u8g2.drawGlyph(19, 13, 0x2126);
    u8g2.setFont(u8g2_font_ncenB08_te);
    u8g2.setCursor(6,13);
    u8g2.print("m");
    u8g2.setCursor(27,13);
    u8g2.print(resistance*1000);
  }
  else if (resistance < lowestResistanceUnsafe) {
    u8g2.setFont(u8g2_font_ncenB08_te);
    u8g2.setCursor(8,13);
    u8g2.print("coil short");
  }


}

void drawPower(float power) {
  u8g2.setFont(u8g2_font_ncenB18_te);
  u8g2.setCursor(8,13);
  u8g2.print(power);
}

void drawMainFrame(void) {
  u8g2.firstPage();
  do {
    u8g2.drawFrame(0,0,128,64);
    drawBattery(batteryVoltage);
    drawResitance(coilResistance);
    drawPower(power);
  } while (u8g2.nextPage());
}


void setup(){

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
SIGNAL(TIMER0_COMPA_vect){

}
//</Interrup>



//<Loop>
void loop(){

  if (batteryVoltage>maxCharchedBattery+1){
    batteryVoltage = lowCriticalBattery;

  }
  else if (batteryVoltage<maxCharchedBattery+1) {
    batteryVoltage+=0.01;
  }

  if (  coilResistance > highestResistance + 1) {
      coilResistance = lowestResistance - 1;
  }
  else {
      coilResistance += 0.1;
  }

  if (  power > maxPower + 20) {
      power = 0;
  }
  else {
      power += 0.1;
  }

  if ((wasSplashScreen == false) && (spalshScreen == true) &&  (millis()-startMillis<=spalshScreenDuration)) {
    getSpalshScreen();
  }
  else if ((wasSplashScreen == false) && (spalshScreen == true) &&  (millis()-startMillis>spalshScreenDuration)) {
    wasSplashScreen = true;
  }
  else {
    drawMainFrame();
  }


}
//</Loop>
