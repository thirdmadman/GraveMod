#ifndef _CONFIG_
#define _CONFIG_
  #define buttonFire 3 // Pin of fire button
  #define buttonUp 4 // Pin of up button
  #define buttonDown 5 // Pin of down button
  #define fireMosfets 9 // Pin of fire mosfets GATE
  #define measureMosfet 8 // Pin of measurement mosfets GATE
  #define measureBattery A2 // Pin (analog) of measurement battery voltage
  #define measureVoltagedivider A1 // Pin (analog) of measurement center point of voltage divider
  #define lowBattery 3.7 // Low battery level after which battery icon will show "!"
  #define lowCriticalBattery 3.4 // Lowest voltage which can be. After that voltage mod will be in unsafe state. (may be will be protection)
  #define overchargedBattery 4.35 // Overcharged battery voltage, use device is highly unsafe
  #define maxCharchedBattery 4.25 // Maximum charged battery voltage, taken as 100% (full) of battery charge icon
  #define mosfetResistance 0.01 // Approximate resistance of fire mosfets
  #define voltagedividerR1 2.7 // Highly precise resistance of R1 in voltage divider
  #define authorName "thirdmadman" // Name of author
  #define version "0.2" // Version name in version control
  #define isDev true // This build is for development?
  #define unsafeMode true // Activate safe mode - HIGHLY NOT RECOMMENDED. USE ON YOUR OWN RISK. (No limits, on safe modes, force and unsafe power)
  #define spalshScreen true // Show spalsh screen with logo and author?
  #define spalshScreenDuration 2000 //Duration of spalsh screen
  #define lowestResistance 0.015 //Minimum coil resistance which can handle mosfets
  #define lowestResistanceUnsafe 0.0001 // //Minimum coil resistance which can handle mosfets (for unsafe mode)
  #define highestResistance 15 // // Maximum coil resistance which can handle battery voltage
  #define startPower 20 // Default power in Watts
  #define startPowerProc 50 // Default power in %
  #define maxPower 300 // Maximum power in Watts which can give mode (may have +50)
  #define powerSaveTimeDef 20000 // Default time after which screen shuts down
  #define timeBeforeFreezeScreen 10000 // Time after which all measurements stops
  #define buttonClickTime 200 // Button click time, @exp
  #define maxPWM 1023 // Maximum PWM in Bits for current config in setup()
  #define VCC 5.005 // Highly precise VCC voltage
#endif
