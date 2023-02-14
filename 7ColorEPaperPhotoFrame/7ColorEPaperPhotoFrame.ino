#include "bmp.h"
#include "epd5in65f.h"
#include <SD.h>
#include <LowPower.h>

/*
Pins Arduino <=> SD Card module
4 - CS / SS (chipSelectSdCard see below)
11 - MOSI
12 - MISO
13 - SCK / CLK
3.3 - 3.3V
GND - GND

Pins Arduino <=> 7 color e-paper display
// see epdif.h for definition of first four
7 - BUSY
8 - RST
9 - DC
6 - CS
13 - CLK
11 - DIN
GND - GND
VCC - 3.3V

BMP image used
- 600 width x 448 height (268800 pixels)
- each byte contains 2 pixels
- most significant nibble left, least significant nibble right
- stored in 268800 / 2 = 134400 bytes
- no run length encoded
- first byte from pixel array is from two first pixels on bottom left
- next byte on the right next to it

Bitmap file header (first 14 bytes it)
size bytes
2    00-01 ID ("BM")
4    02-05 Size of the BMP file
4    06-09 Unused
4    10-13 Offset where the pixel array can be found

Display
- the byte containing the two pixels can be written to the display 'as is'
*/

const int chipSelectSdCard = 4;

int OrH = A3; //orientation switch Horizontal
int OrV = A2; //orientation switch Vertical
int rsp = A1; //pin used to reset the arduino
int SDPWR = 2;  //Pin to power the SD Card reader
bool Vert;
bool Hori;
String FilePath;

  //START

void setup()  
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  pinMode(SDPWR, OUTPUT);

  // wait for Serial Monitor to connect. Needed for native USB port boards only:
  while (!Serial);

  digitalWrite(SDPWR, HIGH);
  //Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelectSdCard))
  {
    Serial.println("SD initialization failed. Things to check:");
    Serial.println("1. is a SD card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");
    while (true);
  }

  //Serial.println("SD initialization done.");

    pinMode(OrH, INPUT_PULLUP);
    pinMode(OrV, INPUT_PULLUP);

    mode(); 

    pinMode(OrH, INPUT);
    pinMode(OrV, INPUT); 

  // Loop over all files in the root folder
  File root = SD.open(FilePath);

  Epd epd;

  //LOOP

  while(true)
  {
    // Init also performs a reset, which will gets the display out of sleep state
    Serial.println("Init display");
    if (epd.Init() != 0)
    {
      Serial.print("e-Paper init failed");
      return;
    }

      
    File file =  root.openNextFile();

    // no more files
    // return to the first file in the directory
    if (!file)
    {
      root.rewindDirectory();
      continue;
    }

    // Filter non bmp files
    if(!String(strlwr(file.name())).endsWith(".bmp"))
    {
      file.close();
      continue;
    }

    //Serial.println("e-Paper Clear");
    epd.Clear(EPD_5IN65F_WHITE);
    /*
    Serial.print("Show: ");
    Serial.println(file.name());
    */
    epd.EPD_5IN65F_Display(&Bmp(&file));
    //Serial.println("Sleep");
    epd.Sleep();

    file.close();

    delay(1000);
    digitalWrite(SDPWR, LOW);
    //Serial.println("power Down (if it would work...)");
    delay(1000);  //delays because the low power library becomes pretty buggy (in my case) without them although it should not
    sleepForduration(64); //duration in s, due it uses the SLEEP_8S command it gets roundet to the nearest multiple of 8
    delay(500);
    pinMode(OrH, INPUT_PULLUP);
    pinMode(OrV, INPUT_PULLUP);

    checkmode ();
  
    pinMode(OrH, INPUT);
    pinMode(OrV, INPUT);
    /*
    digitalWrite(SDPWR, HIGH);
    delay(3000);
    */

  }

  
  //SUBPROGRAMMS


}

void loop() //UNUSED
{
}

void mode() {
  int i = 0;
  while(true) {
    if(digitalRead(OrH)==LOW){
      FilePath = "/hor/";
      Hori = true;
      return;
    }

   if(digitalRead(OrV)==LOW){      
      FilePath = "/ver/";
      Vert = true;
      return;
    }
    delay(100);
      i++;
    if(i>=100){
      FilePath = "/err/";
      return;
      }
    }
  }

  void checkmode () {
    int i = 0;
  while(true) {
    if(digitalRead(OrH)==LOW){
      if (Vert==true){
        pinMode(rsp, OUTPUT);
        digitalWrite(rsp, HIGH);
      }
      return;
    }

   if(digitalRead(OrV)==LOW){      
      if (Hori==true){
        pinMode(rsp, OUTPUT);
        digitalWrite(rsp, HIGH);
      }
      return;
    }
    delay(100);
    i++;
    if(i>=100){
      pinMode(rsp, OUTPUT);
      digitalWrite(rsp, HIGH);
      }

    }
    }

    void sleepForduration(int duration) {
      int cycles = duration/8;
      for (int i=0; i< cycles; i++) {
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
        delay(2);
      }
    }



  