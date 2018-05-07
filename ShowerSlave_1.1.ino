
////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                        //
//              ShowerSlave                                                               //
//                                                                                        //
////////////////////////////////////////////////////////////////////////////////////////////

/* Written by Vebjørn Beinnes for assignment in the course "Prototyping av interactive medier",industrial design, NTNU, Trondheim, Norway.
    Some parts of the code (for Adafruit Mp3 shield) is based on an example for the Adafruit VS1053 Codec Breakout (see text below)
*/

/***************************************************
  This is an example for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
****************************************************/

/* OVERVIEW OF PINS USED:
  Slave - second of two arduinos.

  Digital pin 5 = NeoPixel for indicating points
  0-1 = Serial coms for communicating points between arduinos
  3,4,6,7,8,9,10 = Adafruit mp3-shield
*/

//------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------SETTING VARIABLES!!!

int vol = 10; // volume for L and R channels. lower numbers = higher volume!

//------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------


#include <Adafruit_NeoPixel.h>
int incoming = 0;   // for incoming serial data
int Pin5 = 5;
int w = 0;
int g = 0;

Adafruit_NeoPixel poengLys = Adafruit_NeoPixel(10, Pin5, NEO_GRB + NEO_KHZ800);              //NeoPixel for indicating points
int b = 0;

//-----------------------------------------------------------------------------------------------------------------------------------MUSIC SHIELD DEFINE
//------------------------------------------------------------------------------------------------------------------------------------------------------

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// define the pins used
//#define CLK 13       // SPI Clock, shared with SD card
//#define MISO 12      // Input data, from VS1053/SD card
//#define MOSI 11      // Output data, to VS1053/SD card
// Connect CLK, MISO and MOSI to hardware SPI pins.
// See http://arduino.cc/en/Reference/SPI "Connections"

// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin


Adafruit_VS1053_FilePlayer musicPlayer =
  // create breakout-example object!
  //Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
  // create shield-example object!
  Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);


void setup() {
  Serial.begin(9600);                                            // opens serial port, sets data rate to 9600 bps

  poengLys.begin();
  for (b = 0; b < 10; b++) {

    poengLys.setPixelColor(b, 0, 0, 0);                           //Start by controlling that point ligts are off

    poengLys.show();
  }



  //------------------------------------------------------------------------------------------------------------------------------------MUSIC SHIELD SETUP
  //------------------------------------------------------------------------------------------------------------------------------------------------------

  if (! musicPlayer.begin()) {                                     // initialise the music player
    Serial.println(F("Couldn't find VS1053"));
    while (1);
  }
  Serial.println(F("VS1053 found"));

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }

  musicPlayer.setVolume(vol, vol);                                  //Sets audio output volume to desired value

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT);          // timer int

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);              // DREQ int To be able to perform calculations in the background, while playing shower sound effects

}

void loop() {

  musicPlayer.startPlayingFile("track001.mp3");                     //Starts playing shower sound effects

  while  (musicPlayer.playingMusic) {                               //While music is playing: read current points from master arduino via serial (if available)
    if (Serial.available() > 0) {
      // read the incoming byte:
      incoming = Serial.read(); // poeng 0-10
      int points = incoming - ('0');                                //Because points arrive as a character, subtract character '0' to get correct value

      if (points > 0) {                                             //Controlls lights according to current point value.
        for (b = 0; b < 10; b++) {

          if (b < points) {
            poengLys.setPixelColor(b, 10, 30, 10);
          }
          else {
            poengLys.setPixelColor(b, 0, 0, 0);
          }
          poengLys.show();
        }
      }
      if (points >= 9) {
        poengLys.setPixelColor(9, 10, 30, 10);
        poengLys.setPixelColor(8, 10, 30, 10);
        poengLys.show();
        delay(500);

        points = 0;

        for (g = 0; g < 10; g++) {
          poengLys.setPixelColor(g, 80, 10, 80);
        }
        poengLys.show();
        delay(300);
        for (g = 0; g < 10; g++) {
          poengLys.setPixelColor(g, 0, 0, 0);
        }
        poengLys.show();
        delay(300);

        for (g = 0; g < 10; g++) {
          poengLys.setPixelColor(g, 70, 120, 20);
        }
        poengLys.show();
        delay(300);
        for (g = 0; g < 10; g++) {
          poengLys.setPixelColor(g, 0, 0, 0);
        }
        poengLys.show();
        delay(300);
        for (g = 0; g < 10; g++) {
          poengLys.setPixelColor(g, 15, 0, 80);
        }
        poengLys.show();
        delay(300);
        for (g = 0; g < 10; g++) {
          poengLys.setPixelColor(g, 0, 0, 0);
        }
        poengLys.show();
        delay(300);
      }
    }
  }
}




