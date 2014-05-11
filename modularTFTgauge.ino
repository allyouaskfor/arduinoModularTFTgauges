/*modular gauges...
a new start to the 1.8" tft based gauges
most things are configured in the SD card config file, "gauges.cnf" 

Modular
3 types of gauge
many types of sensor
you choose the order
you choose the peaks/warns/etc.
*/

//There is an issue where only one file can be opened at a time...
//so maybe there is a logging page that you can hit the logging button...

#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <SD.h>

#define SD_CS   4
#define LCD_CS  10
#define LCD_DC  9
#define LCD_RST 8

//int chipSelect = 10; //for adafruit SD shields + tfts
Adafruit_ST7735 tft = Adafruit_ST7735(LCD_CS, LCD_DC, LCD_RST);

File config;
uint16_t background = ST7735_BLACK;
uint16_t outline = ST7735_WHITE;
uint16_t fill = ST7735_BLUE;
uint16_t textdefault = ST7735_RED;
uint16_t alert = ST7735_YELLOW;

void setup() {
	//read some basic settings from the SD card
	Serial.begin(9600);
	tft.initR(INITR_BLACKTAB);
 	tft.fillScreen(background);
	tft.setRotation(1);
	
	Serial.print("SD card start\n");
  	if (!SD.begin(SD_CS)){
    	  Serial.println("failed to initialize SD");
    	  return;
  	}
  	Serial.println("SD OK");
        config = SD.open("gauges");
  	//get name of splash from config file and display it
  	String splash = searchFile("splash");
        //Serial.println("splash"+ splash);
        unsigned int splashLen = splash.length()-1;
        char splashc[splashLen];
        splash.toCharArray(splashc, (splashLen));
        config.close();
  	bmpDraw(splashc, 0, 0);
  	delay(100);

  	//read and assign color configs
        background = textColorToColor(searchFile("background"));
        outline = textColorToColor(searchFile("outline"));
        fill = textColorToColor(searchFile("fill"));
        textdefault = textColorToColor(searchFile("textdefault"));
        alert = textColorToColor(searchFile("alert"));
        
}

void loop() {
  String sensor1, sensor2, sensor3, sensor4;
  String sensor1text, sensor2text, sensor3text, sensor4text;
  unsigned int sensor1pin, sensor2pin, sensor3pin, sensor4pin; //regular sensors get pins...but will check if sensor gets pin of 0...marks obdII
  unsigned int sensor1max, sensor2max, sensor3max, sensor4max;
  unsigned int sensor1alert, sensor2alert, sensor3alert, sensor4alert;
  //read config file for next page
  if (searchFile("pagetype") == "twobar"){//2 sensors displayed in 2 bar charts
    sensor1 = searchFile("sensor1");
    sensor2 = searchFile("sensor2");
    sensor1text = searchFile("sensor1text");
    sensor2text = searchFile("sensor2text");
    sensor1max = searchFile("sensor1max").toInt();
    sensor2max = searchFile("sensor2max").toInt();
    sensor1alert = searchFile("sensor1alert").toInt();
    sensor2alert = searchFile("sensor2alert").toInt();
    //loop to show the display and check for the button press
  }

  else if (searchFile("pagetype") == "onebar"){//1 sensor 1 bar chart...bigger fonts
  }
  
  else if (searchFile("pagetype") == "logging"){//up to 4 sensors shown...log everything to file
  }
  
  else if (searchFile("pagetype") == "round"){//1 sensor 1 round chart
  }
  
  else if (searchFile("pagetype") == "accelerometer"){//cross bar type chart for accelerometer
  }
}

int getSensorReading(String sensorName, int pinNumber){
  //if the pin number is 0 it is obd II...look it up with the obd II lib
  
  //else call the appropriate Analog to digital conversion function on the appropirate pin
  
  return 0;
}

uint16_t textColorToColor(String color){
  if (color == "red"){
    return ST7735_RED;
  }
    else if (color == "magenta"){
    return ST7735_MAGENTA;
  }
    else if (color == "blue"){
    return ST7735_BLUE;
  }
    else if (color == "green"){
    return ST7735_GREEN;
  }
   else if(color == "black"){
    return ST7735_BLACK;
  }
   else  if (color == "white"){
    return ST7735_WHITE;
  }
   else if (color == "yellow"){
    return ST7735_YELLOW;
  }
}

String searchFile(String searchFor){ //finds some substring + : and returns the value after the :
        if (!SD.exists("gauges")){
          Serial.println("config file not found try reformatting sd card");
          return("error");
        }
	//set pos to start of file
	//loop and read a line and check
	String line;
        Serial.println(config.available());
	while (config.available()){
		line = config.readStringUntil('\n');
                if (line.startsWith("#")){ //skip comments
                }
		else if (line.startsWith(searchFor)){
                        config.close();
			int colonPos = line.indexOf(":");
			return( line.substring( line.indexOf(":")+1, line.indexOf("\n")-2 ) );
		}
	}
}

#define BUFFPIXEL 20

void bmpDraw(char *filename, uint8_t x, uint8_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print("File not found");
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print("File size: "); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print("Image Offset: "); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print("Header size: "); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print("Bit Depth: "); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print("Image size: ");
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.Color565(r,g,b));
          } // end pixel
        } // end scanline
        Serial.print("Loaded in ");
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}