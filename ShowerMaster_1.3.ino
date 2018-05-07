
////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                        //
//              ShowerMASTER                                                              //
//                                                                                        //
////////////////////////////////////////////////////////////////////////////////////////////

/* Written by Vebjørn Beinnes for assignment in the course "Prototyping av interactive medier",industrial design, NTNU, Trondheim, Norway.
    Some parts of code for audio frequency recognition is inspired by program by Youtube user "learnelectronics". link: https://www.youtube.com/watch?v=5RmQJtE61zE
*/


/* OVERVIEW OF PINS USED:
  Master - first of two arduinos.

  Digital pin 5 = NeoPixel in shower roof
  Digital pin 11 = NeoPixel for indicating notes
  Analog pin A0 = microphone in shower head
  Analog pin A5 = Sensitivity pot (Adjusted for background noise)
  0-1 = Serial coms for communicating points between arduinos
*/

// Variables for NeoPixel strip-------------------------------------------------------------------------------------------------------------------------

#include <Adafruit_NeoPixel.h>
int Pin5 = 5;
int Pin11 = 11;

int C = 0;
int D = 0;
int E = 0;
int F = 0;
int G = 0;
int A = 0;
int H = 0;

#define NUM_LIGHTS  240


Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LIGHTS, Pin5, NEO_GRB + NEO_KHZ800);      //NeoPixel in shower roof (230 LEDs)
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(8, Pin11, NEO_GRB + NEO_KHZ800);              //NeoPixel for indicating notes
int b = 0;

//------------------------------------------------------------------------------------------------------------------------------------------------------

#include "fix_fft.h"                              //library to perfom the Fast Fourier Transform

/*LINK TO FFT LIBRARY: https://github.com/kosme/arduinoFFT */


//------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------SETTING VARIABLES!!!

int doublecheck = 4; // number of iterations on first loop. higher number = more stable tone recognition, but slower running program
int pointLoopIterations = 8; // How often points drop if no tones are hit by user (Lower number = more often)

//------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------

int potPin = A5;
int Tone = 100;


int freq[64];

char im[128], data[128];                             //variables for the FFT
int Summert[64];                                     //Array for summed values (compares several samplings to minimize effect of audio noise and errors in sampling)
int i = 0, val;                                      //counters
int a = 0;
int z = 0;
int x = 0;
int y = 0; // only for printing to Serial!

int pointLoopCheck = 0;                              //Variables for handling points
int currentPoints = 0;

//--------------------------------------------------------------------------------------------------------------------------------------------VOID SETUP
//------------------------------------------------------------------------------------------------------------------------------------------------------

void setup() {

  Serial.begin(9600);                                 //serial comms for debuging
  analogReference(DEFAULT);                           // Use default (5v) aref voltage.

  pinMode(potPin, INPUT);

  strip.begin();
  strip2.begin();


  strip.show();                                       // Initialize all pixels to 'off'
  strip2.show();


  for (i = 0; i <= 7; i++) {                         //Checks that all LEDs (controlled by master) are working, then turns them all off
    strip2.setPixelColor(i, 20, 100, 20);
    strip2.show();
    delay(10);
  }

  for (i = 0; i < 240 ; i++) {
    strip.setPixelColor(i, 20, 100, 20);
    strip.show();
  }

  for (i = 0; i <= 7; i++) {
    strip2.setPixelColor(i, 0, 0, 0);
    strip2.show();
    delay(10);
  }

  for (i = 0; i < 240 ; i++) {
    strip.setPixelColor(i, 10, 10, 10);
    strip.show();
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------VOID LOOP
//------------------------------------------------------------------------------------------------------------------------------------------------------

void loop() {


  //Serial.println(analogRead(A0));                       //For inspection of microphone readings
  //delay(500);

  int verdi = analogRead(potPin);                         //Maps mic sensitivity from potentiometer
  int sens = map(verdi, 0, 1023, 500, 580);
  //Serial.println(sens);


  if (analogRead(A0) > sens) {                            //Samples from mic, but only if sound louder than the set sensitivity is detected
    memset(Summert, 0, sizeof(Summert));                  //sets all 64 elements in Summert to 0

    for (z = 0; z < doublecheck; z++) {                   // several rounds of sampling to be safe, as some samplings are not perfect (adjusted by variable doublecheck)

      int min = 1024, max = 0;                            //set minumum & maximum ADC values
      for (i = 0; i < 128; i++) {                         //take 128 amplitude samples from microphone
        val = analogRead(A0);                             //get audio from Analog 0 (mic)

        //Serial.print(val);                              //For inspection of raw microphone readings
        //Serial.print("\n");

        data[i] = val / 4 - 128;                          //transforms elements, data[i] is close to zero if silent, +-128 with max sound

        //Serial.print(data[i]);
        //Serial.print("\n");                             //For inspection of elements in the array "data"
        //delay(50);

        im[i] = 0;

      }

      fix_fft(data, im, 7, 0);                            //perform the FFT on data in the array "data"
      for (i = 0; i < 64; i++) {                          // In the current design, 60Hz and noise. (64 is the chosen resolution of the frequency spectrum)

        int dat = sqrt(data[i] * data[i] + im[i] * im[i]);                  //filter out noise and hum from data
        freq[i] = dat;

      }


      //------------------------------------Adds values from "freq" to "Summert"----------------------------------------------------------------------------

      for (x = 0; x < 64; x++) {                          // Adding takes place
        Summert[x] += freq[x];
      }
    }
    //------------------------------------sampling (and adding) loop is done, it's time to discover the frequency-------------------------------------------------
    /*for (y = 0; y < 64; y++) {                            // Test print of summed values, with index
      Serial.print(y);
      Serial.print(" - ");
      Serial.println(Summert[y]);
      }
      delay(1000);
      Serial.print("\n");
      Serial.print("\n");
      Serial.print("\n");
    */
    int maxverdi = 0;
    int maxindex = 0;

    for (z = 0; z < 64; z++) {                              //Saves the index for highest value in array (this index corresponds to a certain frequency span)
      if (Summert[z] > maxverdi) {
        maxverdi = Summert[z];
        maxindex = z;
      }
    }

    //Serial.println(maxindex);                             //for test printing of maxindex
    //delay(500);

    //------------------------------------Assigns tones to value of maxindex--------------------------------------------------------------------------

    if (maxindex != Tone) {                                 // If a NEW note is detected, turn off all LEDs (note-panel and roof)
      for (i = 0; i <= 7; i++) {
        strip2.setPixelColor(i, 0, 0, 0);
      }
      strip2.show();
      for (b = 0; b < 240; b++) {
        strip.setPixelColor(b, 10, 10, 10);
      }
      strip.show();
    }

    if (6 < maxindex < 31) { // If maxindex is not in this range, it is outside the 2 desired octaves. Lower tones are particcularly difficult to measure correctly with this program
      // Program can detect tones from C5 - C7 with reasonable accuracy

      if (maxindex == 7) {
        if (C == 0) {
          strip2.setPixelColor(0, 255, 255, 0);               // Turn on correct LED
          //Serial.print("C");
          //Serial.println();
          for (b = 0; b < 240; b++) {                         // Turn on NeoPixel LEDs in correct color
            strip.setPixelColor(b, 155, 155, 0);
          }
          C = 1;
          D = 0;
          E = 0;
          F = 0;
          G = 0;
          A = 0;
          H = 0;
          if (currentPoints < 10) {                           //Since a note is detected, add 1 to points. (Tried to have one command after if's, common for all notes, but errors often)
            currentPoints += 1;
          }
        }
      }

      //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      if (maxindex == 8 || maxindex == 17) {
        if (D == 0) {

          strip2.setPixelColor(1, 255, 153, 51);
          //Serial.print("D");
          //Serial.println();
          for (b = 0; b < 240; b++) {
            strip.setPixelColor(b, 155, 100, 30);
          }
          C = 0;
          D = 1;
          E = 0;
          F = 0;
          G = 0;
          A = 0;
          H = 0;
          if (currentPoints < 10) {
            currentPoints += 1;
          }
        }
      }
      //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      if (maxindex == 9 || maxindex == 19) {
        if (E == 0) {

          strip2.setPixelColor(2, 255, 0, 255);
          //Serial.print("E");
          //Serial.println();
          for (b = 0; b < 240; b++) {
            strip.setPixelColor(b, 155, 0, 155);
          }
          C = 0;
          D = 0;
          E = 1;
          F = 0;
          G = 0;
          A = 0;
          H = 0;
          if (currentPoints < 10) {
            currentPoints += 1;
          }
        }
      }
      //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      if (maxindex == 10 || maxindex == 20) {
        if (F == 0) {

          strip2.setPixelColor(3, 0, 255, 255);
          //Serial.print("F");
          //Serial.println();
          for (b = 0; b < 240; b++) {
            strip.setPixelColor(b, 0, 155, 155);
          }
          C = 0;
          D = 0;
          E = 0;
          F = 1;
          G = 0;
          A = 0;
          H = 0;
          if (currentPoints < 10) {
            currentPoints += 1;
          }
        }
      }
      //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      if (maxindex == 11 || maxindex == 22) {
        if (G == 0) {

          strip2.setPixelColor(4, 0, 255, 0);
          //Serial.print("G");
          //Serial.println();
          for (b = 0; b < 240; b++) {
            strip.setPixelColor(b, 0, 155, 0);
          }
          C = 0;
          D = 0;
          E = 0;
          F = 0;
          G = 1;
          A = 0;
          H = 0;
          if (currentPoints < 10) {
            currentPoints += 1;
          }
        }
      }
      //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      if (maxindex == 13 || maxindex == 25) {
        if (A == 0) {

          strip2.setPixelColor(5, 255, 0, 0);
          //Serial.print("A");
          //Serial.println();
          for (b = 0; b < 240; b++) {
            strip.setPixelColor(b, 155, 0, 0);
          }
          C = 0;
          D = 0;
          E = 0;
          F = 0;
          G = 0;
          A = 1;
          H = 0;
          if (currentPoints < 10) {
            currentPoints += 1;
          }
        }
      }
      //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      if (maxindex == 14 || maxindex == 28) {
        if (H == 0) {

          strip2.setPixelColor(6, 0, 0, 255);
          //Serial.print("H");
          //Serial.println();
          for (b = 0; b < 240; b++) {
            strip.setPixelColor(b, 0, 0, 155);
          }
          C = 0;
          D = 0;
          E = 0;
          F = 0;
          G = 0;
          A = 0;
          H = 1;
          if (currentPoints < 10) {
            currentPoints += 1;
          }
        }
      }
      //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      if (maxindex == 15) {
        if (C == 0) {

          strip2.setPixelColor(0, 255, 255, 0);
          strip2.setPixelColor(7, 255, 255, 0);
          //Serial.print("C");
          //Serial.println();
          for (b = 0; b < 240; b++) {
            strip.setPixelColor(b, 155, 155, 0);
          }
          C = 1;
          D = 0;
          E = 0;
          F = 0;
          G = 0;
          A = 0;
          H = 0;
          if (currentPoints < 10) {
            currentPoints += 1;
          }
        }
      }
      //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

      if (maxindex == 30) {
        if (C == 0) {

          strip2.setPixelColor(0, 255, 255, 0);
          //Serial.print("C");
          //Serial.println();
          for (b = 0; b < 240; b++) {
            strip.setPixelColor(b, 155, 155, 0);
          }
          C = 1;
          D = 0;
          E = 0;
          F = 0;
          G = 0;
          A = 0;
          H = 0;
          if (currentPoints < 10) {
            currentPoints += 1;
          }
        }
      }

      strip.show();
      strip2.show();

      //Serial.println("Tone index: ");
      //Serial.println(maxindex);

      //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    }
    Tone = maxindex;
  }

  else {                                                                // If no sound (over set sensitivity) is detected for a while, run function "noTone" (resets lights)
    Notone();
  }

  Serial.println(currentPoints);                                        //Communicates current point score to slave arduino via serial.
  if (currentPoints >= 9) {                                            //If max points reached, run rainbow ligt program from function
    currentPoints = 0;                                                  //...Then reset points
    Serial.println(currentPoints);
    theaterChaseRainbow(50);

  }

  //Serial.print("Points: ");
  //Serial.println(currentPoints);
  //Serial.println();



}

void Notone() {                                                         // Function for turning off Tone-LEDs and NeoPixel in roof
  int verdi = analogRead(potPin);
  int sens = map(verdi, 0, 1023, 500, 580);
  if (analogRead(A0) < sens) {
    delay(50);
    if (analogRead(A0) < sens) {
      delay(50);
      if (analogRead(A0) < sens) {
        delay(50);
        if (analogRead(A0) < sens) {
          delay(50);
          if (analogRead(A0) < sens) {
            delay(50);


            if (analogRead(A0) < sens) {
              for (i = 0; i <= 7; i++) {
                strip2.setPixelColor(i, 0, 0, 0);
                //Serial.print("Alle tone-LED av!");
              }
              for (b = 0; b < 240; b++) {
                strip.setPixelColor(b, 10, 10, 10);
              }
              strip.show();
              strip2.show();

              pointLoopCheck += 1;
              //Serial.println(pointLoopCheck);

              if (pointLoopCheck >= pointLoopIterations) {           // Subtracts one point from score every so often (timing adjusted by pointLoopCheck variable)
                //Serial.println("points decrease");

                if (currentPoints > 0) {                             //Don't subtract if score is zero
                  currentPoints = (currentPoints - 1);

                  pointLoopCheck = 0;                                //Reset resetting counter

                }
              }
            }
          }
        }
      }
    }
  }
}

void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 25; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

