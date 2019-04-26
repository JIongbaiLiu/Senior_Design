#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>       // this is needed for display
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Adafruit_FT6206.h>
#include "RTClib.h"


#define TFT_CS 10
#define TFT_DC 9
#define BOXSIZE 10
#define BOLD 1
#define HOME_PAGE 0
#define SETTINGS_PAGE 1
#define TIME_SETTINGS_PAGE 2
#define SET_POINTS_PAGE 3

// general vars
int current_page = HOME_PAGE;
const char* dayNames[7] = {"Sun", "Mon", "Tues", "Wed", "Thu", "Fri", "Sat"};
const char* am_pm[2] = {"AM", "PM"};
RTC_DS3231 rtc;
DateTime Clock;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_FT6206 ts = Adafruit_FT6206();

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
int current_day = 0;
int previous_day = current_day;
int current_hour = 6;
int previous_hour = current_hour;
int current_minute = 5;
int previous_minute = current_minute;


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

  //---------sandbox space-----------
  drawTimeSettingPage();
  current_page = TIME_SETTINGS_PAGE;
  //---------------------------------

//  drawHomeScreen();
}

void loop() {
  //TODO: get real_temp
  //TODO: get time from RTC (or EEPROM??)
  
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
          // Go Back
          if(p.x >= BOXSIZE * 0 && p.x <= BOXSIZE * 5 && p.y >= BOXSIZE * 22 && p.y <= BOXSIZE * 32) {
            current_page = HOME_PAGE;
            clearScreen();
            drawHomeScreen();
            break;
          }

          // Day & Time
          if(p.x >= BOXSIZE * 7 && p.x <= BOXSIZE * 12 && p.y >= BOXSIZE * 10 && p.y <= BOXSIZE * 25) {
            current_page = TIME_SETTINGS_PAGE;
            clearScreen();
            drawTimeSettingPage();
            break;
          }

          // Set Points
          if(p.x >= BOXSIZE * 15 && p.x <= BOXSIZE * 20 && p.y >= BOXSIZE * 10 && p.y <= BOXSIZE * 25) {
            current_page = SET_POINTS_PAGE;
            clearScreen();
            drawSetPointsPage();
            break;
          }
          
        case TIME_SETTINGS_PAGE:
        // day up arrow
        if(p.x >= BOXSIZE * 6 && p.x <= BOXSIZE * 8 && p.y >= BOXSIZE * 23 && p.y <= BOXSIZE * 26) {
          previous_day = current_day;
          if(current_day < 6) {
            current_day++;
          }
          else {
            current_day = 0;
          }
          // update day label
          updateText(BOXSIZE * 3, BOXSIZE * 11.5, 2, String(dayNames[previous_day]), String(dayNames[current_day]));
          break;
        }

        // day down arrow
        if(p.x >= BOXSIZE * 14 && p.x <= BOXSIZE * 16 && p.y >= BOXSIZE * 23 && p.y <= BOXSIZE * 26) {
          previous_day = current_day;
          if(current_day >= 1) {
            current_day--;
          }
          else {
            current_day = 6;
          }
          // update day label
          updateText(BOXSIZE * 3, BOXSIZE * 11.5, 2, String(dayNames[previous_day]), String(dayNames[current_day]));
          break; 
        }

        // hour up arrow
        if(p.x >= BOXSIZE * 6 && p.x <= BOXSIZE * 8 && p.y >= BOXSIZE * 13 && p.y <= BOXSIZE * 16) {
          if(!(current_hour > 11)) {
            previous_hour = current_hour;
            current_hour++;
            float ref_x;
            if(current_hour > 9 && previous_hour != 9) {
              ref_x = 15;
            }
            // special case when going from 9 to 10
            else if(current_hour > 9 && previous_hour == 9){
                updateText(BOXSIZE * 16.5, BOXSIZE * 11.5, BOXSIZE * 15, 2, String(previous_hour), String(current_hour));
                break;
            }
            else {
              ref_x = 16.5;
            }
            updateText(BOXSIZE * ref_x, BOXSIZE * 11.5, 2, String(previous_hour), String(current_hour));
          }
          break;
        }

        // hour down arrow
        if(p.x >= BOXSIZE * 14 && p.x <= BOXSIZE * 16 && p.y >= BOXSIZE * 13 && p.y <= BOXSIZE * 16) {
          if (current_hour > 1) {
            previous_hour = current_hour;
            current_hour--;
            
            float ref_x;
            if(current_hour < 10 && previous_hour != 10) {
              ref_x = 16.5;
            }
            // special case when going from 10 to 9
            else if(current_hour < 10 && previous_hour == 10) {
              updateText(BOXSIZE * 15, BOXSIZE * 11.5, BOXSIZE * 16.5, 2, String(previous_hour), String(current_hour));
              break;
            }
            else {
              ref_x = 15;
            }
            updateText(BOXSIZE * ref_x, BOXSIZE * 11.5, 2, String(previous_hour), String(current_hour));
          }
          break;
        }

        // minute up arrow
        if(p.x >= BOXSIZE * 6 && p.x <= BOXSIZE * 8 && p.y >= BOXSIZE * 7 && p.y <= BOXSIZE * 10) {
          break;
        }

        // minute down arrow
        if(p.x >= BOXSIZE * 14 && p.x <= BOXSIZE * 16 && p.y >= BOXSIZE * 7 && p.y <= BOXSIZE * 10) {
          break;
        }
      }
    }
    currently_touched = true;
  }
}

/**
 * These methods draw the five pages
 *  - HOME_PAGE
 *  - SETTINGS_PAGE
 *  - TIME_SETTINGS_PAGE
 *  - SET_POINTS_PAGE
 */

// TODO: could probably refactor to be more like settings page with more printText calls
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
  printText(BOXSIZE * 16, BOXSIZE *  2.5, 1, "Settings", ILI9341_WHITE);

  // first button
  tft.drawRect(BOXSIZE *  7, BOXSIZE * 7, BOXSIZE * 15, BOXSIZE * 5, ILI9341_WHITE);
  printText(BOXSIZE * 9, BOXSIZE * 10, 1, "Day & Time", ILI9341_WHITE);

  // second button
  tft.drawRect(BOXSIZE * 7, BOXSIZE * 15, BOXSIZE * 15, BOXSIZE * 5, ILI9341_WHITE);
  printText(BOXSIZE * 9, BOXSIZE * 18, 1, "Set Points", ILI9341_WHITE);
}

void drawTimeSettingPage() {
  // Day label
  printText(BOXSIZE * 5, BOXSIZE * 2, 1, "Day", ILI9341_WHITE);

  // Time Label
  printText(BOXSIZE * 18.5, BOXSIZE * 2.5, 1, "Time", ILI9341_WHITE);

  // toggling arrows
  drawArrows(BOXSIZE * 5, BOXSIZE * 7.5);   // day
  drawArrows(BOXSIZE * 16, BOXSIZE * 7.5);  // hour
  drawArrows(BOXSIZE * 22, BOXSIZE * 7.5);  // minute

  // day
  //updateText(BOXSIZE * 3, BOXSIZE * 11.5, 2, String(dayNames[previous_day]), String(dayNames[current_day]));
  printText(BOXSIZE * 3, BOXSIZE * 11.5, 2, String(dayNames[current_day]), ILI9341_WHITE);

  // hour
  if(current_hour > 9) {
    printText(BOXSIZE * 15, BOXSIZE * 11.5, 2, String(current_hour), ILI9341_WHITE);
  }
  else {
    printText(BOXSIZE * 16.5, BOXSIZE * 11.5, 2, String(current_hour), ILI9341_WHITE);
  }
  
  // minute
  if(current_minute > 9) {
    printText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String(current_minute), ILI9341_WHITE);
  }
  else {
    printText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String ("0") + String(current_minute), ILI9341_WHITE);
  }

  // time colon
  tft.fillRect(203, 95, 4, 4, ILI9341_WHITE);
  tft.fillRect(203, 110, 4, 4, ILI9341_WHITE);

  // am vs pm
  printText(BOXSIZE * 27, BOXSIZE * 8.5, 1, "AM", ILI9341_WHITE);
  printText(BOXSIZE * 27, BOXSIZE * 13.5, 1, "PM", ILI9341_WHITE);

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

// TODO: this
void drawSetPointsPage() {
  printText(BOXSIZE * 7, BOXSIZE * 10, 1, "Under Construction!", ILI9341_WHITE);
}

/**
 * These helpers methods draw home screen items
 */

// clear screen
void clearScreen() {
  tft.fillScreen(ILI9341_BLACK);
}

// prints text according to loose design patterns of this app
void printText(int x_start, int y_start, int text_size, String text, uint16_t color) {
  tft.setCursor(x_start, y_start);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(color);
  tft.setTextSize(text_size);
  tft.print(text);
}

void printText(int x_start, int y_start, int text_size, String text, uint16_t color, bool Bold) {
  tft.setFont(&FreeSansBold9pt7b);
  tft.setCursor(x_start, y_start);
  tft.setTextColor(color);
  tft.setTextSize(text_size);
  tft.print(text);
}

// writes over current text in the background color, then writes again in original color
void updateText(int x_start, int y_start, int text_size, String prev_text, String new_text) {
  // "clear" what is there now
  printText(x_start, y_start, text_size, prev_text, ILI9341_BLACK);

  // set new text
  printText(x_start, y_start, text_size, new_text, ILI9341_WHITE);
}

// specifically for text where the prev_text & new_text are at different start coords (e.g. 9, 10)
void updateText(int prev_x_start, int y_start, int new_x_start, int text_size, String prev_text, String new_text) {
  // "clear" what is there now
  printText(prev_x_start, y_start, text_size, prev_text, ILI9341_BLACK);

  // set new text
  printText(new_x_start, y_start, text_size, new_text, ILI9341_WHITE);
}

// TODO: updateText for bold fonts


// modular corner button 
void drawCornerButton(String label) {
  tft.drawLine(BOXSIZE * 0, BOXSIZE * 4, BOXSIZE * 10, BOXSIZE * 4, ILI9341_WHITE);
  tft.drawLine(BOXSIZE * 10, BOXSIZE * 4, BOXSIZE * 10, BOXSIZE * 0, ILI9341_WHITE);
  printText(BOXSIZE * 1.5, BOXSIZE * 2, 1, label, ILI9341_WHITE);
}

// contains mode, auto status, hold status
void drawBottomBar() {
  // horiz line
  tft.drawLine(0, 200, 320, 200, ILI9341_WHITE);

  // Cool/Heat status
  printText(BOXSIZE, BOXSIZE * 22.5, 1, "Cool Mode", ILI9341_WHITE);

  // Auto status
  String label;
  uint16_t color;
  if(auto_on) {
    // first vert line
    tft.drawLine(115, 201, 115, 240, ILI9341_BLACK);
    
    // draw inverted box
    tft.fillRect(116, 201, 99, 40, ILI9341_WHITE);
    color = ILI9341_BLACK;
    label = "Auto on";
  }
  else {
    // first vert line
    tft.drawLine(115, 201, 115, 240, ILI9341_WHITE);
    tft.fillRect(116, 201, 99, 40, ILI9341_BLACK);
    color = ILI9341_WHITE;
    label = "Auto off";
  }
  printText(BOXSIZE * 13.5, BOXSIZE * 22.5, 1, label, color);

  // Hold status
   tft.setCursor(235, 225);
  if(hold_on) {
    // second vert line
    tft.drawLine(215, 201, 215, 240, ILI9341_BLACK);
    tft.fillRect(216, 201, 109, 39, ILI9341_WHITE);
    color = ILI9341_BLACK;
    label = "Hold on";
  }
  else {
    // second vert line
    tft.drawLine(215, 201, 215, 240, ILI9341_WHITE);
    tft.fillRect(216, 201, 109, 39, ILI9341_BLACK);
    color = ILI9341_WHITE;
    label = "Hold off";
  }
  printText(BOXSIZE * 23.5, BOXSIZE * 22.5, 1, label, color);
}

// Prints the current temperature
void printCurrentTemp(){
  // "clear" what is there now
  printText(BOXSIZE * 8, BOXSIZE * 14.5, 4, String(prev_real_temp), ILI9341_BLACK, BOLD);
  printText(BOXSIZE * 16.5, BOXSIZE * 10.5, 1, "o", ILI9341_BLACK, BOLD);
  printText(BOXSIZE * 17.7, BOXSIZE * 12, 2, "F", ILI9341_BLACK, BOLD);

  // set new temp
  printText(BOXSIZE * 8, BOXSIZE * 14.5, 4, String(real_temp), ILI9341_WHITE, BOLD);
  printText(BOXSIZE * 16.5, BOXSIZE * 10.5, 1, "o", ILI9341_WHITE, BOLD);
  printText(BOXSIZE * 17.7, BOXSIZE * 12, 2, "F", ILI9341_WHITE, BOLD);
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
  //updateTimeHomePage();
  printText(BOXSIZE * 20, BOXSIZE * 1.5, 1, "Wed, 12:00AM", ILI9341_WHITE);
}

// Prints the set temperature
void printSetTemp() {
  updateText(BOXSIZE * 26, BOXSIZE * 13, 2, String(prev_set_temp), String(set_temp));
}

//void updateTimeHomePage() {
//  Clock = rtc.now();
//  String dateTime = String(dayNames[Clock.dayOfTheWeek()]) + String(", ") + String(Clock.hour())
//                    + String(":") + String(Clock.minute()) + String(am_pm[am_selected]);
//  printText(BOXSIZE * 20, BOXSIZE * 1.5, 1, dateTime, ILI9341_WHITE);
//}
