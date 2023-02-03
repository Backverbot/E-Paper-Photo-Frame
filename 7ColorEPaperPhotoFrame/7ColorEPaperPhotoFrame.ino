#include "bmp.h"
#include "epd5in65f.h"
#include <SD.h>

/*
Pins Arduino <=> SD Card module
10 - CS / SS (chipSelectSdCard see below)
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

int OrH = 2; //orientation switch Horizontal
int OrV = 3; //orientation switch Vertical

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  // wait for Serial Monitor to connect. Needed for native USB port boards only:
  while (!Serial);

  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelectSdCard))
  {
    Serial.println("SD initialization failed. Things to check:");
    Serial.println("1. is a SD card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");
    while (true);
  }

  Serial.println("SD initialization done.");
    

    String FolderPath = String(""); //just to create it for the open function to not freak out

  
  for (bool fori = true; fori==true;){
    
    pinMode(OrH, INPUT_PULLUP);
    pinMode(OrV, INPUT_PULLUP); 

    if(digitalRead(OrH)==LOW){
      String FolderPath = String("/hor/");
      Serial.println(FolderPath);
       fori = false;
    }

   if(digitalRead(OrV)==LOW){      
      String FolderPath = String("/ver/");
      Serial.println(FolderPath);
       fori = false;
    }
    delay(100);
  }

  
  // Loop over all files in the root folder
  File root = SD.open(FolderPath);

  Epd epd;

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

    Serial.println("e-Paper Clear");
    epd.Clear(EPD_5IN65F_WHITE);
    Serial.print("Show: ");
    Serial.println(file.name());
    epd.EPD_5IN65F_Display(&Bmp(&file));
    Serial.println("Sleep");
    epd.Sleep();

    file.close();

    Serial.println("start delay");
    delay(120000);  // 2 minutes
  
  }
}

void loop()
{
}
