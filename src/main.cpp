#include <Arduino.h>
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define buttonFire 3
#define buttonUp 4
#define buttonDown 5
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
#define version "0.1"
#define isDev true
#define powerLimit 250
#define unsafeMode true
#define minCoilResistance 0.01
#define maxCoilResistance 10
#define spalshScreen true
#define spalshScreenDuration 2000
#define lowestResistance 0.015
#define lowestResistanceUnsafe 0.0001
#define highestResistance 15
#define startPower 20
#define maxPower 300
#define powerSaveTimeDef 20000



bool wasSplashScreen = false;
float batteryVoltage = 2,
coilResistance = 0,
powerSaveTime = powerSaveTimeDef;


class Mod {
private:
  float power = startPower;
public:
  unsigned long startMillis = 0;
  unsigned long lastActive = 0;

  void setPower(float setPower) {
    if (setPower <= maxPower && setPower >=0) {
      power = setPower;
    }

  }
  float getPower() {
    return power;
  }
};
Mod graveMod;


class Coil {
private:
  bool fire = false;
public:
  float getCoilResistance() {
    return 0.04;
  }
  void setFire(bool state) {
    fire = state;
    if (fire == true) {
      digitalWrite(fireMosfets, HIGH);
    }
    else {
      digitalWrite(fireMosfets, LOW);
    }
  }
  bool getFireState() {
    return fire;
  }
  void stateCorrection() {
    if (getFireState()) {
      digitalWrite(fireMosfets, HIGH);
    }
    else {
      digitalWrite(fireMosfets, LOW);
    }

  }
};

Coil coil;

class Button {
private:
  int pinNumber;
  bool pressed = false;
  bool down = false;
  unsigned long rearMillis = 0;
  unsigned long frontMillis = 0;
public:
  Button(int pin) {
    pinNumber = pin;
  }

  bool getDownState() {
    return down;
  }
  bool getPressedState() {
    return pressed;
  }
  unsigned long getLastPressed() {
    return millis() - rearMillis;
  }
  unsigned long getPressTime() {
    return millis() - frontMillis;
  }
  void readState() {
    if (digitalRead(pinNumber)==HIGH ) {
      if (pressed == false) {
        pressed = true;
        down = true;
        frontMillis = millis();
        graveMod.lastActive = millis();
      }
    }
    else if (digitalRead(pinNumber)==LOW) {
      if (pressed == true) {
        pressed = false;
        down = false;
        rearMillis = millis();
        graveMod.lastActive = millis();
      }
    }
  }

};

Button bFire(buttonFire);
Button bUp(buttonUp);
Button bDown(buttonDown);

class Battery {
public:
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

};

Battery Battery;

class UI {
private:
  bool powerSave = false;
public:

  void setPowerSave(bool state) {
    if (powerSave != state) {
      if (state == true ) {
        u8g2.setPowerSave(1);
        powerSave = true;
      }
      else {
        u8g2.setPowerSave(0);
        powerSave = false;
      }
    }
  }

  bool poweSaveState() {
    return powerSave;
  }

  void drawSpalshScreen(void) {

    u8g2.drawStr(35,12, "Grave mod");
    u8g2.drawStr(42,24, "made by");
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.drawStr(10,42, authorName);
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(36,56, "version");
    u8g2.drawStr(80,56, version);
  }

  void drawBattery(float voltage) {

    String state = Battery.getBatteryState(voltage);

    if (state=="BATTERY_OVERCHARGED") {
      //don't let user use this battery
      u8g2.drawFrame(96,4,25,10);
      u8g2.drawBox(121, 6, 3, 6);
      u8g2.drawStr(104,13,"!!!");
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
      u8g2.setCursor(72,13);
      u8g2.print(voltage);
      u8g2.drawStr(109,13,"!");
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
      u8g2.drawStr(108,13,"?");
    }
    u8g2.setCursor(72,13);
    u8g2.print(voltage);
  }

  void drawResitance(float resistance) {

    //float setResistance = resitace + mosfetResis;
    //u8g2.print(setResistance);

    if (resistance > highestResistance) {
      u8g2.setCursor(8,13);
      u8g2.print("coil none");
    }
    else if (resistance >= 0.01) {
      u8g2.setFont(u8g2_font_6x12_t_symbols);
      u8g2.drawGlyph(19, 13, 0x2126);
      u8g2.setFont(u8g2_font_ncenB08_tr);

      u8g2.setCursor(27,13);
      u8g2.print(resistance);
    }
    else if (resistance < 0.01 && resistance > lowestResistanceUnsafe) {
      u8g2.setFont(u8g2_font_6x12_t_symbols);
      u8g2.drawGlyph(19, 13, 0x2126);
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.setCursor(6,13);
      u8g2.print("m");
      u8g2.setCursor(27,13);
      u8g2.print(resistance*1000);
    }
    else if (resistance < lowestResistanceUnsafe) {
      u8g2.setCursor(8,13);
      u8g2.print("coil short");
    }


  }

  void drawPower(float power) {
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.setCursor(8,36);
    u8g2.print(power);
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }

  void drawMainScreen() {
    u8g2.drawFrame(0,0,128,64);
    drawBattery(batteryVoltage);
    drawResitance(coilResistance);
    drawPower(graveMod.getPower());
  }

  void drawMainFrame(void) {
    if (millis()-graveMod.lastActive < powerSaveTime) {
      setPowerSave(false);
      if ((wasSplashScreen == false) && (spalshScreen == true) &&  (millis()-graveMod.startMillis<=spalshScreenDuration)) {
        drawSpalshScreen();
      }
      else if ((wasSplashScreen == false) && (spalshScreen == true) &&  (millis()-graveMod.startMillis>spalshScreenDuration)) {
        wasSplashScreen = true;
      }
      else {
        drawMainScreen();
      }
    }
    else if (millis()-graveMod.lastActive >= powerSaveTime) {
      setPowerSave(true);
    }
  }
};

UI Ui;



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
  graveMod.startMillis=millis();
  u8g2.setFont(u8g2_font_ncenB08_tr);

}

//<Interrup>
SIGNAL(TIMER0_COMPA_vect){
  bFire.readState();
  coil.stateCorrection();
  bUp.readState();
  bDown.readState();
}
//</Interrup>



//<Loop>
void loop(){
  u8g2.firstPage();
  do {

    if (bFire.getDownState() == true) {
      if ( bFire.getPressTime() >= 150) {
        u8g2.drawStr(30, 54, "1");
        coil.setFire(true);
      }
      else {
        coil.setFire(false);
      }

    }
    else if (bUp.getDownState() == true && bUp.getPressTime()>=100) {
        graveMod.setPower(graveMod.getPower()+0.01);
    }
    else if (bDown.getDownState() == true && bDown.getPressTime()>=100) {
        graveMod.setPower(graveMod.getPower()-0.01);
    }

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


    Ui.drawMainFrame();
  } while (u8g2.nextPage());
}
//</Loop>
