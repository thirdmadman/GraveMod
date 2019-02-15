#include <Arduino.h>
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#include "config.h"

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result/1000; // Vcc in millivolts
}

class Mod {
private:
  float power = startPower;
  float powerProc = startPowerProc;
  int powerManagementMode = 0;
public:
  float vcc = VCC;
  unsigned long startMillis = 0;
  unsigned long lastActive = 0;

  void setPower(float setPower) {
    if (powerManagementMode == 1) {
      if (setPower <= maxPower && setPower >=0) {
        power = setPower;
      }
    } else if (powerManagementMode == 0) {
      if (setPower <= 100 && setPower >=0) {
        power = setPower;
      }
    }
  }

  float getPower() {
    return power;
  }

  void setPowerManagementMode(int modeN) {
    powerManagementMode = modeN;
  }

  int getPowerManagementMode() {
    return powerManagementMode;
  }
};
Mod graveMod;

class IO {
public:

  float readFromPin(int pinNumber, int arryLenth) {
    float read[arryLenth];
    for (int i = 0; i < arryLenth - 1; i++) {
      read[i] = analogRead(pinNumber);
    }

    for (int i = 0; i < arryLenth - 1; i++) {
      for (int j = 0; j < arryLenth - i - 1; j++) {
        if (read[j] > read[j + 1]) {
          float temp = read[j];
          read[j] = read[j + 1];
          read[j + 1] = temp;
        }
      }
    }

    float outFloat = 0;
    int spector = 6;
    for (int i = (((arryLenth/2)-(spector/2))-1); i < (((arryLenth/2)+(spector/2))-1); i++) {
      outFloat += read[i];
    }
    outFloat /= spector;
    return outFloat;
  }

};

IO io;

class Battery {
private:
  float voltage = 0;
  IO io;
public:

  int getBatteryState(float volts) {

    if (voltage>maxCharchedBattery) {
      //don't let user use this battery
      //return "BATTERY_OVERCHARGED";
      return 1;
    }
    else if ((voltage<=maxCharchedBattery) && (voltage>lowBattery)) {
      //return "BATTERY_NORMAL";
      return 2;
    }
    else if ((voltage<=lowBattery) && (voltage>lowCriticalBattery)) {
      //notice - battery is low
      //return "BATTERY_LOW";
      return 3;
    }
    else if ((voltage<=lowCriticalBattery) && (voltage>(lowCriticalBattery-1))) {
      //don't let user discharge battery more
      //return "BATTERY_LOWCRITICAL";
      return 4;

    }
    else if (voltage<=(lowCriticalBattery-1)) {
      //check your battery - is it there?
      //return "BATTERY_NONE";
      return 5;
    }

    return 0;

  }

  float readBatteryVoltagePrec() {
    voltage = io.readFromPin(measureBattery, 50) * (graveMod.vcc / 1023.0);
    //voltage = io.readFromPin(measureBattery, 50);
    return voltage;
  }

  float readBatteryVoltage() {
    voltage = io.readFromPin(measureBattery, 10) * (graveMod.vcc / 1023.0);
    //voltage = io.readFromPin(measureBattery, 10);
    return voltage;
  }

  float getBatteryVoltage() {
    return voltage;
  }

};

Battery Battery;

class Coil {
private:
  bool fire = false;
  float coilResistance = lowestResistanceUnsafe;
  int PWM = maxPWM/2;
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
      if (graveMod.getPowerManagementMode() == 0){
        int powerIn = floor(graveMod.getPower()*10.23);
        PWM = powerIn;
        analogWrite(fireMosfets, PWM);
      } else if (graveMod.getPowerManagementMode() == 1) {

      }
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
      if (graveMod.getPowerManagementMode() == 0){
        int powerIn = floor(graveMod.getPower()*10.23);
        PWM = powerIn;
        analogWrite(fireMosfets, PWM);
      } else if (graveMod.getPowerManagementMode() == 1) {

      }
    }
    else {
      digitalWrite(fireMosfets, LOW);
    }
  }

  float readResitance() {
    //fucking matherfucking safety first!
    if (getFireState() == false) {
      digitalWrite(measureMosfet, HIGH);
      delay(10); // let it wait until transition states gone
      float resReadFloat =  io.readFromPin(measureVoltagedivider,50) * (graveMod.vcc / 1023.0);
      float precBatteryValtage = Battery.readBatteryVoltagePrec();
      digitalWrite(measureMosfet, LOW);
      coilResistance = (voltagedividerR1*(precBatteryValtage - resReadFloat))/resReadFloat;
    }
    return coilResistance;
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
  unsigned long forgetClicksTime = buttonClickTime * 1.5 ;
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
    if (digitalRead(pinNumber)==HIGH) {
      if (pressed == false) {
        pressed = true;
        down = true;
        frontMillis = millis();
        graveMod.lastActive = millis();
        clicksCount++;

      }
    }
    else if (digitalRead(pinNumber)==LOW) {
      if ((pressed == true) && (getPressTime() >= 10)) {
        pressed = false;
        down = false;
        rearMillis = millis();
        graveMod.lastActive = millis();
        if (getPressTime() <= buttonClickTime) {
          clicked = true;
        }
      }
      if (millis()-rearMillis>=forgetClicksTime && clicksCount > 0) {
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
  int getClicks() {
    if (clicks > 0) {
      int outR = clicks;
      clicks = 0;
      return outR;
    }
    return 0;
  }

};

Button bFire(buttonFire);
Button bUp(buttonUp);
Button bDown(buttonDown);

class MenuList {
private:
  String menuPositions[6];
  bool selectedPosition[6];
  int currentMenuSize = 0;
  int currentMenuPosition = 0;
public:
  void addListPosition(String menuName) {
    menuPositions[currentMenuSize] = menuName;
    selectedPosition[currentMenuSize] = false;
    currentMenuSize++;
  }
  void nexListPosition() {
    if (currentMenuPosition < currentMenuSize-1) {
      currentMenuPosition++;
    }

  }
  void prevListPosition() {
    if (currentMenuPosition > 0) {
      currentMenuPosition -=1;
    }
  }
  void drawList() {
    int printPoseY =12;
    for (int i = 0; i < currentMenuSize; i++) {
      u8g2.setCursor(5, printPoseY);
      u8g2.print(menuPositions[i]);
      printPoseY += 12;
    }
    u8g2.drawFrame(3, (currentMenuPosition*12)+1, 120, 14);
    u8g2.setCursor(72,13);
    u8g2.print(currentMenuSize);

  }
  void setSelectedPosition(bool setter) {
    for (int i = 0; i < currentMenuSize; i++) {
      selectedPosition[i] = false;
    }
    selectedPosition[currentMenuPosition] = setter;
  }
  int getSelectedPosition() {
    for (int i = 0; i < currentMenuSize; i++) {
    if (selectedPosition[i]==true){
      return i;
    }
    }
    return -1;
  }
};

MenuList mainMenu;
MenuList fireModeMenu;

class Menu {
private:
  MenuList lists[2];
  int currentList = 0;
public:

  Menu() {
    mainMenu.addListPosition("Fire mode");
    mainMenu.addListPosition("Max fire time");
    mainMenu.addListPosition("Full power");
    mainMenu.addListPosition("Screen save time");
    mainMenu.addListPosition("About mod");
    fireModeMenu.addListPosition("<-");
    fireModeMenu.addListPosition("Power in %");
    fireModeMenu.addListPosition("Power in Watts");
    lists[0] = mainMenu;
    lists[1] = fireModeMenu;
  }
  int getCurrentList() {
    return currentList;
  }

  void nexMenuPosition() {
    lists[currentList].nexListPosition();

  }
  void prevMenuPosition() {
    lists[currentList].prevListPosition();
  }
  void drawMenu() {
    if ((currentList == 0) && (lists[currentList].getSelectedPosition() == 0)) {
      currentList = 1;
    }
    else if ((currentList == 1) && (lists[currentList].getSelectedPosition() == 0)) {
      lists[currentList].setSelectedPosition(false);
      currentList = 0;
      lists[currentList].setSelectedPosition(false);
    }
    lists[currentList].drawList();

  }
  void setSelectedPosition(bool setter) {
    lists[currentList].setSelectedPosition(setter);
  }

  int getSelectedPosition() {
    return lists[currentList].getSelectedPosition();
  }
};

Menu menu;

class UI {
private:
  bool wasSplashScreen = false;
  bool powerSave = false;
  int drawMode = 1;
  // 1 = MAIN_FRAME
  // 2 = MENU
  float powerSaveTime = powerSaveTimeDef;
public:

  UI() {

  }

  void drawPowerModMenu() {

  }

  void drawMaxFireTimeFrame() {
    u8g2.drawStr(35,12, "Max fire time");
  }

  void drawScreenSaveTimeFrame() {
    u8g2.drawStr(35,12, "Screen save time");
  }

  void drawAboutModFrame() {
    u8g2.drawStr(35,12, "About mod");
  }

  void setPowerSaveTime(float time) {
    powerSaveTime = time;
  }

  float getPowerSaveTime() {
    return powerSaveTime;
  }

  void setDrawMode(int option) {
    drawMode = option;

  }

  int getDrawMode() {
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

    if (Battery.getBatteryState(voltage)==1) {
      //don't let user use this battery
      u8g2.drawFrame(96,4,25,10);
      u8g2.drawBox(121, 6, 3, 6);
      u8g2.drawStr(104,13,"!!!");
      u8g2.setCursor(72,13);
      u8g2.print(voltage);
    }
    else if (Battery.getBatteryState(voltage)==2) {
      u8g2.drawFrame(96,4,25,10);
      u8g2.drawBox(121, 6, 3, 6);
      int batteryPix = floor((voltage - lowBattery) / ((maxCharchedBattery - lowBattery) / 25.0));
      if (batteryPix > 25) {
        batteryPix = 25;
      }
      u8g2.drawBox(97, 5, batteryPix, 8);
    }
    else if (Battery.getBatteryState(voltage)==3) {
      //notice - battery is low
      u8g2.drawFrame(96,4,25,10);
      u8g2.drawBox(121, 6, 3, 6);
      u8g2.setCursor(72,13);
      u8g2.print(voltage);
      u8g2.drawStr(109,13,"!");
    }
    else if (Battery.getBatteryState(voltage)==4) {
      //don't let user discharge battery more
      u8g2.drawFrame(96,4,25,10);
      u8g2.drawBox(121, 6, 3, 6);
      u8g2.drawLine(96,4,120,13);
      u8g2.drawLine(96,13,120,4);
    }
    else if (Battery.getBatteryState(voltage) == 5) {
      //check your battery - is it there?
      u8g2.drawStr(108,13,"?");
    }
    if (voltage>0 &&  voltage < overchargedBattery+1) {
      u8g2.setCursor(72,13);
      u8g2.print(voltage);
    }

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
    if (graveMod.getPowerManagementMode() == 0) {
      u8g2.drawStr(70,36,"%");
    } else if (graveMod.getPowerManagementMode() == 1) {
      u8g2.drawStr(70,36,"W");
    }

    u8g2.setFont(u8g2_font_ncenB08_tr);
  }

  void drawMainScreen() {
    drawBattery(Battery.getBatteryVoltage());
    drawResitance(coil.getCoilResistance());
    drawPower();
  }

  void drawMenu() {
    if (menu.getCurrentList() == 0) {
      if (menu.getSelectedPosition() == 1) {
        drawMaxFireTimeFrame();
      }
      else if (menu.getSelectedPosition() == 2) {
        if (graveMod.getPowerManagementMode() == 0){
          graveMod.setPower(100);
        } else if (graveMod.getPowerManagementMode() == 1) {
          menu.setSelectedPosition(false);
        }
      }
      else if (menu.getSelectedPosition() == 3) {
        drawScreenSaveTimeFrame();
      }
      else if (menu.getSelectedPosition() == 4) {
        drawAboutModFrame();
      }
      else {
          menu.drawMenu();
      }
    }


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
      else if (drawMode == 2) {
        drawMenu();
      }
      else if (drawMode == 1) {
        drawMainScreen();
      }
      else {
        u8g2.setCursor(35,13);
        u8g2.print(drawMode);
        u8g2.drawFrame(0,0,128,64);
        u8g2.setCursor(72,13);
        u8g2.print("Warning!");
      }

    }
    else if (millis()-graveMod.lastActive >= powerSaveTime) {
      setPowerSave(true);
    }
  }

};

UI Ui;


void setup() {

  //<Interrupts>
  // that's making interrupts about once a second
  //OCR0A - comparison register for Timer 0
  //OCR0A = 0xAF; //175
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  //TIMSK0 - Timer Interrupt Mask Register
  //OCIE0A - Output Compare Match Interrupt Enable A bit

  //</Interrupts>

  //<PWM>
  //that's making PWM 10 bit (1024) on 15625 Hz
  TCCR1A = TCCR1A & 0xe0 | 3; //224
  TCCR1B = TCCR1B & 0xe0 | 0x09; //
  //</PWM>
  if (isDev == true) {
    Serial.begin(9600); // Use fore debug?
  }
  pinMode(measureMosfet, OUTPUT);
  pinMode(fireMosfets, OUTPUT);
  pinMode(buttonFire, INPUT);
  pinMode(buttonUp, INPUT);
  pinMode(buttonDown,INPUT);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  graveMod.startMillis=millis();
}

//<Interrup>
SIGNAL(TIMER0_COMPA_vect) {
  bFire.readState();
  //coil.stateCorrection();
  bUp.readState();
  bDown.readState();
}
//</Interrup>



//<Loop>
void loop() {
  if (bFire.getDownState() == true) {
    if ( bFire.getPressTime() >= 150 && (Ui.getDrawMode() != 2)) {
      coil.setFire(true);
    } else if ((Ui.getDrawMode() == 2) && (bFire.getPressTime() >= 1000)) {
        Ui.setDrawMode(1);
        menu.setSelectedPosition(false);
    }
  }
  else if (bFire.getDownState() == false) {
    coil.setFire(false);
  }


  if (millis() - graveMod.lastActive < timeBeforeFreezeScreen) {
    if (coil.getFireState()) {
      Battery.readBatteryVoltagePrec();
    } else {
      Battery.readBatteryVoltage();
      coil.readResitance();
    }
  }


  if ((bUp.buttonClicked()) || (bUp.getPressTime()>=1000 && bUp.getDownState() == true)) {
    if ((Ui.getDrawMode() != 2)) {
      graveMod.setPower(graveMod.getPower()+0.5);
    } else if (Ui.getDrawMode() == 2) {
      menu.nexMenuPosition();
    }
  }
  else if ((bDown.buttonClicked()) || (bDown.getPressTime()>=1000 && bDown.getDownState() == true)) {
    if ((Ui.getDrawMode() != 2)) {
      graveMod.setPower(graveMod.getPower()-0.5);
    } else if (Ui.getDrawMode() == 2) {
      menu.prevMenuPosition();
    }
  }
  else if (bFire.buttonClicked()) {
    if (Ui.getDrawMode() == 2) {
      menu.setSelectedPosition(true);
    }
  }

  if (bFire.getClicks() >=5) {
    Ui.setDrawMode(2);
  }

  u8g2.firstPage();
  do {
    Ui.drawMainFrame();
  } while (u8g2.nextPage());
}
//</Loop>
