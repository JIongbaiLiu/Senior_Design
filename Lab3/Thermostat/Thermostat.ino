#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>


#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void setup() {
  
  tft.begin();
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
  // put your main code here, to run repeatedly:

}

void printMode(){
  tft.setCursor(75, 85);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Cooling");
  tft.print(" to");
}

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

void printSetTempArrows(){
  int x_start = 210;
  int y_start = 115;

  tft.fillTriangle(x_start, y_start, x_start+15, y_start-20, x_start+30, y_start, ILI9341_WHITE);  //x0, y0, x1, y1, x2, y2, color
  tft.fillTriangle(x_start, y_start+20, x_start+15, y_start+40, x_start+30, y_start+20, ILI9341_WHITE);  //x0, y0, x1, y1, x2, y2, color
}

void printDOWandTime(){
  tft.setCursor(200, 15);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Wed, 12:00AM");
}

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

void printCurrentTemp() {
  tft.setCursor(40, 30);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("75 ");
  tft.print("F");
  tft.setCursor(80, 12);
  tft.setTextSize(1);
  tft.print("o");

}
