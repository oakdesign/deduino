//**********************************************************************//
//  Name    : DEDuino, Arduino displays for FalconBMS                   //
//  Author  : Uri Ben-Avraham                                           //
//  Date    : 20 Jan, 2017                                              //
//  Version : 1.3.0-alpha1                                              //
//  License : MIT                                                       //
//  Notes   : Uncomment the DEFINE for the Arduino board in use         //
//          : Boards supported by this version:                         //
//          : Arduino Uno, Arduino Micro & Arduino Due                  //
//          : All Common Config options are found under "config.h"      //
//          :                                                           //
//          : Uncomment the DEFINE for the features you wish to use     //
//          : Features included in this version:                        //
//          : Displays: DED, FFI, PFD                                   //
//          : Lights: Indexers, Caution Panel                           //
//**********************************************************************//

#include <Arduino.h>
#include "config.h"
//*******************************************************************//
//  All configuration options are set via the "config.h" file        //
//  Please DO NOT edit code below this segment                       //
//  (Unless you know what you are doing - in which case.. have fun!) //
//*******************************************************************//


///*************** DO NOT EDIT BELOW THIS LINE WITHOUT INTENT! ***************//
///********** ALL COMMON CONFIG OPTIONS ARE FOUND THE config.h FILE **********//
#define MICRO_DELAY delayMicroseconds(250);
#ifdef ARDUINO_UNO
//////////////////////////////////////////////////////////////////////////
//  Arduino Uno                                                         //
//  SCK - Pin 13                                                        //
//  MISO - Pin 12 (not used in this project)                            //
//  MOSI - Pin 11                                                       //
//  SS - Pin10 (set to output and pulled high on setup)                 //
//////////////////////////////////////////////////////////////////////////
// Define SPI Pins
#define SCK 13
#define MISO 12
#define MOSI 11
#define SS 10
#endif

#ifdef ARDUINO_MICRO
//////////////////////////////////////////////////////////////////////////
//  Arduino Micro                                                       //
//  SCK, MISO, MOSI - all on dedicated pins no defines needed           //
//  use the Arduino Micro Pinout for reference                          //
//  https://www.arduino.cc/en/uploads/Main/ArduinoMicro_Pinout3.png     //
//////////////////////////////////////////////////////////////////////////
#endif

#ifdef ARDUINO_DUE
//////////////////////////////////////////////////////////////////////////
//  Arduino Due                                                         //
//  SCK, MISO, MOSI - all located on the ICSP header                    //
//  Only MOSI and SCK needs to be connected via the ICSP                //
//                                                                      //
//     1 - MISO | 0 0 | 2 - VCC                                         //
//     3 - SCK  | 0 0 | 4 - MOSI                                        //
//     5 - Reset| 0 0 | 6 - GND                                         //
//                                                                      //
//  NOTE:                                                               //
//  * Arduino Due has TWO (2) ICSP headers,                             //
//    Use the once closer to the ARM chip (not next to the power jack)  //
//  * Connect the Arduino Due using the "Native USB" port               //
//////////////////////////////////////////////////////////////////////////
#endif

///////////////////
//// Includes ////
//////////////////

// Serial Comm
#include "comms.h"

// displays
#ifdef Screens
  #include "U8g2lib.h"
  
  #ifdef SpeedBreaks_on
    #include "speedbreaks_img.h"
    #include "speedbreaks.h"
  #endif
  
  #if defined DED_on || defined PFD_on
    #include "falconded_u8g2.h"
  #endif

  #ifdef DED_on
    #include "ded.h"
  #endif

  #ifdef PFD_on
    #include "pfd.h"
  #endif
  
  #ifdef CMDS_on
    #include "falconded_u8g2.h"
    #include "cmds.h"
  #endif

  #ifdef FuelFlow_on
    #include "fuelflow_u8g2.h"
    #include "fuelflow.h"
  #endif
#endif

// LightPanels general config
// Light General
#ifdef Lights
  #ifdef USE_SPI
    #include <SPI.h>
    #include "lights_spi.h"
  #endif
  #ifdef USE_I2C
    #include <Wire.h>
    #include "lights_i2c.h"
  #endif
#endif

#include "internal.h"
///////////////////////////
//// Global Variables  ////
///////////////////////////

short Run = 0;

////////////////////
//// Functions ////
///////////////////

/////// Main Program /////////////
void setup() {
  // Init SPI/WIRE
#ifdef Lights
  #ifdef USE_SPI
    #ifdef ARDUINO_UNO
      pinMode(SS, OUTPUT);
      digitalWrite(SS, HIGH);
    #endif
    SPI.begin();
    #ifdef ARDUINO_DUE
      SPI.setBitOrder(MSBFIRST);
      SPI.setClockDivider(10);
    #else
      SPI.setBitOrder(LSBFIRST);
      SPI.setClockDivider(SPI_CLOCK_DIV2);
    #endif
  #endif

  #ifdef USE_I2C
    Wire.begin();
  #endif
#endif

delay(2000); // to allow screens to boot on power on defore init.

initSerial();

// Initiallize displays - but be from small to large
#ifdef FuelFlow_on
  initFF();
#endif
#ifdef SpeedBreaks_on
  initSB();
#endif

#ifdef DED_on
  initDED();
#endif
#ifdef PFD_on
  initPFD();
#endif

#ifdef CMDS_on
  initCMDS();
#endif


#ifdef Lights
  initLights();
#endif

 delay(POST_BOOT_BIT); // To allow user to verify everything is working correcly
}

void loop() {
  if (gotoSleep) {
    goDark();
  }
  
  // Fuel Flow
#ifdef FuelFlow_on
  readFF();
  drawFF();
#endif

  //DED
#ifdef DED_on
  readDED();
  drawDED();
#endif

  // Fuel Flow (again) - for refresh rate
#ifdef FuelFlow_on
  readFF();
  drawFF();
#endif
  // Indexers
#ifdef Indexers_on
  readAOA();
  lightAOA();
#endif

  // Non refresh critical function, run those alternating every loop.
  switch (Run) {
    case 0:
#ifdef PFD_on
      readPFD();
      drawPFD();
#endif
#ifdef Glareshield_on
      // Caution panel
      readGlareShield();
      lightGlareshield();
#endif
      Run = 1;
      break;
    case 1:
#ifdef CautionPanel_on
      // Caution panel
      readCautionPanel();
      lightCautionPanel();
#endif
#ifdef CMDS_on
      readCMDS();
      drawCMDS();
#endif
      Run = 2;
      break;
    case 2:
#ifdef SpeedBreaks_on
      readSB();
      drawSB();
#endif
      Run = 0;
      break;
  }
}
