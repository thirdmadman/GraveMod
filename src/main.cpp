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
#define minCoilResistance 0.001
#define maxCoilResistance 10
#define spalshScreen true
#define spalshScreenDuration 2000
#define lowestResistance 0.015
#define lowestResistanceUnsafe 0.0001
#define highestResistance 15
#define startPower 20
#define maxPower 300
#define powerSaveTimeDef 20000
#define buttonClickTime 200



bool wasSplashScreen = false;
float batteryVoltage = 2,
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
  float coilResistance = lowestResistanceUnsafe;
public:
  void setCoilReistace(float resistance) {
    coilResistance = resistance;
  }
  float getCoilResistance() {
    return coilResistance;
  }
  void setFire(bool state) {
    fire = state;
    if (fire) {
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
    if (fire) {
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
  bool clicked = false;
  int clicksCount = 0;
  int clicks = 0;
  int forgetClicksTime = buttonClickTime * 1.5;
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
        clicksCount++;
      }
    }
    else if (digitalRead(pinNumber)==LOW) {
      if (pressed == true) {
        pressed = false;
        down = false;
        rearMillis = millis();
        graveMod.lastActive = millis();
        if (getPressTime() <= buttonClickTime) {
          clicked = true;
        }
      }
      if (millis()-rearMillis>=forgetClicksTime) {
        clicks = clicksCount;
        clicksCount = 0;
      }
    }
  }
  bool buttonClicked () {
    if (clicked) {
      clicked = false;
      return true;
    }
    else return false;
  }
  int getClicks(){
    if (clicks > 0) {
      int out = clicks;
      clicks = 0;
      return out;
    }
    return clicks;
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
  String drawMode = "MAIN_FRAME";
public:

  void setDrawMode(String option) {
    drawMode = option;
  }

  String getDrawMode() {
    return drawMode;
  }

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
    else if (resistance >= 0.1) {
      u8g2.setFont(u8g2_font_6x12_t_symbols);
      u8g2.drawGlyph(34, 13, 0x2126);
      u8g2.setFont(u8g2_font_ncenB08_tr);

      u8g2.setCursor(4,13);
      u8g2.print(resistance);
    }
    else if (resistance < 0.1 && resistance > lowestResistanceUnsafe) {
      u8g2.setFont(u8g2_font_6x12_t_symbols);
      u8g2.drawGlyph(46, 13, 0x2126);
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.setCursor(34,13);
      u8g2.print("m");
      u8g2.setCursor(4,13);
      u8g2.print(resistance*1000);
    }
    else if (resistance < lowestResistanceUnsafe) {
      u8g2.setCursor(8,13);
      u8g2.print("coil short");
    }


  }

  void drawPower(void) {
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.setCursor(4,36);
    u8g2.print(graveMod.getPower());
    u8g2.drawStr(70,36,"W");
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }

  void drawMainScreen() {
    u8g2.drawFrame(0,0,128,64);
    drawBattery(batteryVoltage);
    drawResitance(coil.getCoilResistance());
    drawPower();
  }

  void drawMenu() {
    u8g2.drawFrame(0,0,128,64);
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
      else if (drawMode == "MENU") {
        drawMenu();
      }
      else if (drawMode == "MAIN_FRAME") {
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
  if (isDev == true && Serial.available() > 0) {
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
  if (bFire.getClicks() >=3) {
    Ui.setDrawMode("MENU");
  }
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
        Ui.setDrawMode("MAIN_FRAME");
      }
    }
    else if (bFire.getDownState() == false) {
      coil.setFire(false);
    }

    if ((bUp.buttonClicked()) || (bUp.getPressTime()>=1000 && bUp.getDownState() == true)) {
        graveMod.setPower(graveMod.getPower()+0.5);
    }
    else if ((bDown.buttonClicked()) || (bDown.getPressTime()>=1000 && bDown.getDownState() == true)) {
        graveMod.setPower(graveMod.getPower()-0.5);
    }

    if (batteryVoltage>maxCharchedBattery+1){
      batteryVoltage = lowCriticalBattery;
    }
    else if (batteryVoltage<maxCharchedBattery+1) {
      batteryVoltage+=0.01;
    }

    if (coil.getCoilResistance() > highestResistance + 1) {
      coil.setCoilReistace(lowestResistanceUnsafe - 1);
    }
    else {
      coil.setCoilReistace(coil.getCoilResistance()+0.001);
    }

    Ui.drawMainFrame();
  } while (u8g2.nextPage());
}
//</Loop>
