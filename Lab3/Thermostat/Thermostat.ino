#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>       // this is needed for display
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Adafruit_FT6206.h>


#define TFT_CS 10
#define TFT_DC 9

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_FT6206 ctp = Adafruit_FT6206();

int temp = 75;

void setup() {
  
  while (!Serial);     // used for leonardo debugging
 
  Serial.begin(115200);
  Serial.println(F("Cap Touch Paint!"));
  
  tft.begin();

  if (! ctp.begin(40)) {  // pass in 'sensitivity' coefficient
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }

  Serial.println("Capacitive touchscreen started");
  
  tft.fillScreen(ILI9341_BLACK);

  //rotate
  uint8_t rotation = 3;
  tft.setRotation(rotation);

  printMode();
  printSetTemp();
  printSetTempArrows();
  printDOWandTime();
  drawHouse();
  printCurrentTemp();

}

void loop() {
  //get temp
  // update temp,, if changed or not??

  if(ctp.touched()) { 
    // Retrieve a point  
    TS_Point p = ctp.getPoint();

    // flip it around to match the screen.
    p.x = map(p.x, 0, 240, 240, 0);
    p.y = map(p.y, 0, 320, 320, 0);

    
  }

}

// Prints the mode: AC, Heat, Auto
void printMode(){
  tft.setCursor(75, 85);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Cooling");
  tft.print(" to");
}

// Prints the set point temp
void printSetTemp(){
  tft.setCursor(75, 150);
  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextSize(4);
  tft.println("72");
  tft.setCursor(160, 110);
  tft.setTextSize(1);
  tft.println("o");
  tft.setCursor(172, 125);
  tft.setTextSize(2);
  tft.println("F");
}

// Prints the temp toggling arrows
void printSetTempArrows(){
  int x_start = 210;
  int y_start = 115;

  tft.fillTriangle(x_start, y_start, x_start+15, y_start-20, x_start+30, y_start, ILI9341_WHITE);  //x0, y0, x1, y1, x2, y2, color
  tft.fillTriangle(x_start, y_start+20, x_start+15, y_start+40, x_start+30, y_start+20, ILI9341_WHITE);  //x0, y0, x1, y1, x2, y2, color
}

// Prints the Day of Week and current time
void printDOWandTime(){
  tft.setCursor(200, 15);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Wed, 12:00AM");
}

// Draws the house icon
void drawHouse(){
  // roof
  int roof_x = 15;
  int roof_y = 0;
  tft.drawLine(roof_x, roof_y, roof_x-15, roof_y+15, ILI9341_WHITE);
  tft.drawLine(roof_x, roof_y, roof_x+15, roof_y+15, ILI9341_WHITE);

  // rest of house
  int house_x = 6;
  int house_y = 12;
  tft.drawLine(house_x, house_y, house_x, house_y+20, ILI9341_WHITE);
  tft.drawLine(house_x+18, house_y, house_x+18, house_y+20, ILI9341_WHITE);
  tft.drawLine(house_x, house_y+20, house_x+18, house_y+20, ILI9341_WHITE);
}

// Prints the current temperature
void printCurrentTemp() {
  tft.setCursor(40, 30);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print(temp);
  tft.print(" ");
  tft.print("F");
  tft.setCursor(80, 12);
  tft.setTextSize(1);
  tft.print("o");

}
