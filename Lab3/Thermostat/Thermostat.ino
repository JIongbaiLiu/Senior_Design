#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>       // this is needed for display
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Adafruit_FT6206.h>


#define TFT_CS 10
#define TFT_DC 9

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_FT6206 ts = Adafruit_FT6206();

int real_temp = 75;
int prev_real_temp = real_temp;
int set_temp = 72;
int prev_set_temp = set_temp;
bool currently_touched = false;
bool auto_on = true;
bool hold_on = true;

void setup() {
  
  while (!Serial);
 
  Serial.begin(115200);
  
  tft.begin();

  if (! ts.begin(40)) {  // pass in 'sensitivity' coefficient
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }

  Serial.println("Capacitive touchscreen started");
  
  tft.fillScreen(ILI9341_BLACK);

  //rotate
  uint8_t rotation = 3;
  tft.setRotation(rotation);

  
  
//  printMode();
  printSetTemp();
  printSetTempArrows();
  printDOWandTime();
  printCurrentTemp();
  drawBottomBar();
}

void loop() {
  //get real_temp
  // update real_temp,, if changed or not??
  if(!ts.touched()){
    currently_touched = false;
    return;
  }

  if(ts.touched()) {
      if(!currently_touched) {
      // Retrieve a point  
      TS_Point p = ts.getPoint();

      // flip it around to match the screen.
      p.x = map(p.x, 0, 240, 240, 0);
      p.y = map(p.y, 0, 320, 320, 0);

      // up arrow
      if (p.x >= 210 && p.x <= 230 && p.y >= 95 && p.y <= 115) {
        prev_set_temp = set_temp;
        set_temp = 44;
        printSetTemp();
      }
    }
    currently_touched = true;
  }
}

/**
 * Helper functions
 */

// prints the main home screen 
void printHomeScreen() {
//  printMode();
  printSetTemp();
  printSetTempArrows();
  printDOWandTime();
  printCurrentTemp();
  drawBottomBar();
}


void drawBottomBar() {

  // horiz line
  tft.drawLine(0, 200, 320, 200, ILI9341_WHITE);

  // Cool/Heat status
  tft.setCursor(10, 225);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Cool Mode");

  // first vert line
  tft.drawLine(115, 201, 115, 240, ILI9341_BLACK);

  // Auto status
  tft.setCursor(135, 225);
  if(auto_on) {
    // draw inverted Auto
    tft.fillRect(116, 201, 99, 40, ILI9341_WHITE);

    
    tft.setTextColor(ILI9341_BLACK);
    tft.print("Auto on");
  }
  else {
    tft.setTextColor(ILI9341_WHITE);
    tft.print("Auto off");
  }

  // second vert line
  tft.drawLine(215, 201, 215, 240, ILI9341_BLACK);

  // Hold status
   tft.setCursor(235, 225);
  if(hold_on) {
    tft.fillRect(216, 201, 109, 39, ILI9341_WHITE);
    tft.setTextColor(ILI9341_BLACK);
    tft.print("Hold on");
  }
  else {
    tft.setTextColor(ILI9341_WHITE);
    tft.print("Hold off");
  }
}

// slated for deletion 
//// Prints the mode: AC, Heat, Auto
//void printMode(){
//  tft.setCursor(75, 85);
//  tft.setFont(&FreeSans9pt7b);
//  tft.setTextColor(ILI9341_WHITE);
//  tft.setTextSize(1);
//  tft.print("Cooling");
//  tft.print(" to");
//}

// Prints the current temperature
void printCurrentTemp(){
  int x_start = 80;
  int y_start = 145;
  
  // "clear" what is there now
  tft.setCursor(x_start, y_start);
  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(4);
  tft.println(prev_real_temp);
  tft.setCursor(x_start+85, y_start-40);
  tft.setTextSize(1);
  tft.println("o");
  tft.setCursor(x_start+97, y_start-25);
  tft.setTextSize(2);
  tft.println("F");

  // set new temp
  tft.setCursor(x_start, y_start);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(4);
  tft.println(real_temp);
  tft.setCursor(x_start+85, y_start-40);
  tft.setTextSize(1);
  tft.println("o");
  tft.setCursor(x_start+97, y_start-25);
  tft.setTextSize(2);
  tft.println("F");
}

// Prints the temp toggling arrows
void printSetTempArrows() {
  int x_start = 265;
  int y_start = 90;

  // up
  tft.fillTriangle(x_start, y_start, x_start+15, y_start-20, x_start+30, y_start, ILI9341_WHITE);  //x0, y0, x1, y1, x2, y2, color

  // down
  tft.fillTriangle(x_start, y_start+60, x_start+15, y_start+80, x_start+30, y_start+60, ILI9341_WHITE);  //x0, y0, x1, y1, x2, y2, color
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
  // rest of house
  int house_x = 11;
  int house_y = 6;
  tft.drawLine(house_x, house_y, house_x, house_y+10, ILI9341_WHITE);
  tft.drawLine(house_x+9, house_y, house_x+9, house_y+10, ILI9341_WHITE);
  tft.drawLine(house_x, house_y+10, house_x+8, house_y+10, ILI9341_WHITE);
  
  // roof
  tft.drawLine(house_x+4, house_y-6, house_x-3, house_y+1, ILI9341_WHITE);
  tft.drawLine(house_x+4, house_y-6, house_x+11, house_y+1, ILI9341_WHITE);
}

// Prints the set temperature
void printSetTemp() {
  int start_x = 260;
  int start_y = 130;
  
  // "clear" what is there now
  tft.setCursor(start_x, start_y);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.print(prev_set_temp);


  // set new temp
  tft.setCursor(start_x, start_y);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print(set_temp);
}
