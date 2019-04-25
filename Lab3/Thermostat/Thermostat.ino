#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>       // this is needed for display
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Adafruit_FT6206.h>


#define TFT_CS 10
#define TFT_DC 9
#define BOXSIZE 10
#define HOME_PAGE 0
#define SETTINGS_PAGE 1
#define TIME_SETTINGS_PAGE 2

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_FT6206 ts = Adafruit_FT6206();


// general vars
int current_page = HOME_PAGE;
const char* dayNames[] = {"Sun", "Mon", "Tues", "Wed", "Thu", "Fri", "Sat"};

// home page vars
int real_temp = 75;
int prev_real_temp = real_temp;
int set_temp = 72;
int prev_set_temp = set_temp;
bool currently_touched = false;
bool auto_on = false;
bool hold_on = false;

// time setting page vars
bool am_selected = false;


void setup() {
  
  while (!Serial);
 
  Serial.begin(115200);
  
  tft.begin();

  if (! ts.begin(40)) {  // pass in 'sensitivity' coefficient
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }

  Serial.println("Capacitive touchscreen started");
  
  clearScreen();

  //rotate
  uint8_t rotation = 3;
  tft.setRotation(rotation);

  // sandbox space

  drawTimeSettingPage();
  // drawHomeScreen();
}

void loop() {
  //TODO: get real_temp
  
  if(!ts.touched()){
    currently_touched = false;
    return;
  }

  if(ts.touched()) {
      if(!currently_touched) {
      // Retrieve a point  
      TS_Point p = ts.getPoint();

      // flip it around to match the screen.
      // map(value, fromLow, fromHigh, toLow, toHigh)
      // TODO: fix this, axis of TS_Point is different than the LCD's 
      p.x = map(p.x, 0, 240, 240, 0);
      p.y = map(p.y, 0, 320, 320, 0);

      switch(current_page) {
        case HOME_PAGE:
          // settings button
          if(p.x >= BOXSIZE * 0 && p.x <= BOXSIZE * 4 && p.y >= BOXSIZE * 22 && p.y <= BOXSIZE * 32) {
            current_page = SETTINGS_PAGE;
            clearScreen();
            drawSettingsScreen();
            break;
          }

          // auto button
          if (p.x >= BOXSIZE * 20 && p.x <= BOXSIZE * 24 && p.y >= BOXSIZE * 10.5 && p.y <= BOXSIZE * 20.5) {
            auto_on = !auto_on;
            drawBottomBar();
            break;
          }

          // hold button
          if(p.x >= BOXSIZE * 20 && p.x <= BOXSIZE * 24 && p.y >= BOXSIZE * 0 && p.y <= BOXSIZE * 10) {
            hold_on = !hold_on;
            drawBottomBar();
            break;
          }

          // up arrow
          if (p.x >= BOXSIZE * 9 && p.x <= BOXSIZE * 11 && p.y >= BOXSIZE * 2.5 && p.y <= BOXSIZE * 5.5) {
            if (!(set_temp > 89)) {
              prev_set_temp = set_temp;
              set_temp++;
              printSetTemp();
              break;
            }
          }

          // down arrow
          if (p.x >= BOXSIZE * 17 && p.x <= BOXSIZE * 19 && p.y >= BOXSIZE * 2.5 && p.y <= BOXSIZE * 5.5) {
            if (!(set_temp < 41)) {
              prev_set_temp = set_temp;
               set_temp--;
               printSetTemp();
               break;
            }
          }
          
        case SETTINGS_PAGE:
          if(p.x >= BOXSIZE * 0 && p.x <= BOXSIZE * 4 && p.y >= BOXSIZE * 22 && p.y <= BOXSIZE * 32) {
            current_page = HOME_PAGE;
            clearScreen();
            drawHomeScreen();
            break;
          }
      }
      
    }
    currently_touched = true;
  }
}

/**
 * Screen drawing methods. Some of these call other helper methods below
 */

// prints the main home screen 
void drawHomeScreen() {
  drawCornerButton("Settings");
  printSetTemp();
  drawArrows(265, 90);
  printDOWandTime();
  printCurrentTemp();
  drawBottomBar();
}

void drawSettingsScreen() {
  drawCornerButton("Go Back");

  // title
  tft.setCursor(BOXSIZE * 16, BOXSIZE *  2.5);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Settings");

  // first button
  tft.drawRect(BOXSIZE *  7, BOXSIZE * 7, BOXSIZE * 15, BOXSIZE * 5, ILI9341_WHITE);
  tft.setCursor(BOXSIZE * 9, BOXSIZE * 10);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Day & Time");

  // second button
  tft.drawRect(BOXSIZE * 7, BOXSIZE * 15, BOXSIZE * 15, BOXSIZE * 5, ILI9341_WHITE);
  tft.setCursor(BOXSIZE * 9, BOXSIZE * 18);
  tft.print("Set Points");
}

void drawTimeSettingPage() {
  // Day label
  tft.setCursor(BOXSIZE * 5, BOXSIZE * 2);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Day");

  // Time Label
  tft.setCursor(BOXSIZE * 18.5, BOXSIZE * 2.5);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Time");

  // toggling arrows
  drawArrows(BOXSIZE * 5, BOXSIZE * 7.5);
  drawArrows(BOXSIZE * 16, BOXSIZE * 7.5);
  drawArrows(BOXSIZE * 22, BOXSIZE * 7.5);

  // day
  tft.setCursor(30, 115);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Wed");

  // hour
  tft.setCursor(150, 115);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("12");

  // minute
  tft.setCursor(215, 115);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("00");

  
  // time colon
  tft.fillRect(203, 95, 4, 4, ILI9341_WHITE);
  tft.fillRect(203, 110, 4, 4, ILI9341_WHITE);

  // REDO WITH SIZE 2 FONT
  // am vs pm
  tft.setCursor(270, 85);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("AM");
  /*------------------------------*/
  tft.setCursor(270, 135);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("PM");

  // selected line
  if (am_selected) {
    tft.fillRect(270, 90, 27, 2, ILI9341_WHITE);
  }
  else {
    tft.fillRect(270, 140, 27, 2, ILI9341_WHITE);
  }

  // save button
  tft.drawRect(45, 190, 85, 35, ILI9341_WHITE);
  printText(68, 213, 1, "Save", ILI9341_WHITE);
  
  // cancel button
  tft.drawRect(165, 190, 85, 35, ILI9341_WHITE);
  printText(178, 213, 1, "Cancel", ILI9341_WHITE);
}

/**
 * These helpers methods draw home screen items
 */

// clear screen
void clearScreen() {
  tft.fillScreen(ILI9341_BLACK);
}

// prints text according to standards within the context of this app
void printText(int x_start, int y_start, int text_size, String text, uint16_t color) {
  tft.setCursor(x_start, y_start);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(color);
  tft.setTextSize(text_size);
  tft.print(text);
}

// modular corner button 
void drawCornerButton(String label) {
  tft.drawLine(BOXSIZE * 0, BOXSIZE * 4, BOXSIZE * 10, BOXSIZE * 4, ILI9341_WHITE);
  tft.drawLine(BOXSIZE * 10, BOXSIZE * 4, BOXSIZE * 10, BOXSIZE * 0, ILI9341_WHITE);
  tft.setCursor(15, 20);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print(label);
}

// contains mode, auto status, hold status
void drawBottomBar() {
  // horiz line
  tft.drawLine(0, 200, 320, 200, ILI9341_WHITE);

  // Cool/Heat status
  tft.setCursor(10, 225);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Cool Mode");


  // Auto status
  tft.setCursor(135, 225);
  if(auto_on) {
    // first vert line
    tft.drawLine(115, 201, 115, 240, ILI9341_BLACK);
    // draw inverted Auto
    tft.fillRect(116, 201, 99, 40, ILI9341_WHITE);
    tft.setTextColor(ILI9341_BLACK);
    tft.print("Auto on");
  }
  else {
    // first vert line
    tft.drawLine(115, 201, 115, 240, ILI9341_WHITE);
    tft.fillRect(116, 201, 99, 40, ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("Auto off");
  }

  // Hold status
   tft.setCursor(235, 225);
  if(hold_on) {
    // second vert line
    tft.drawLine(215, 201, 215, 240, ILI9341_BLACK);
    tft.fillRect(216, 201, 109, 39, ILI9341_WHITE);
    tft.setTextColor(ILI9341_BLACK);
    tft.print("Hold on");
  }
  else {
    // second vert line
    tft.drawLine(215, 201, 215, 240, ILI9341_WHITE);
    tft.fillRect(216, 201, 109, 39, ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("Hold off");
  }
}

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

// draws two arrows: one up, one down
void drawArrows(int x_start, int y_start) {
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
