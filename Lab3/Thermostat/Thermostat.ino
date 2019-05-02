#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>       // this is needed for display
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Adafruit_FT6206.h>
#include "RTClib.h"
#include <EEPROM.h>


#define TFT_CS 10
#define TFT_DC 9
#define BOXSIZE 10
#define BOLD 1
#define ARROW_WIDTH 30
#define ARROW_HEIGHT 20
#define ARROWS_OFFSET 60
#define WEEKDAY 0
#define WEEKEND 1
#define NOT_SET 255
//do i need these?
#define COOL_MODE 0
#define HEAT_MODE 1
#define ATUO_MODE 2


// pages
#define HOME_PAGE 0
#define SETTINGS_PAGE 1
#define TIME_SETTINGS_PAGE 2
#define SET_POINTS_PAGE 3
#define EDIT_SET_POINT_PAGE 4


/**
 * Class Name: Set Point
 * Description: Holds the set point data
 * 
 * Note: The "constructor" is set_values. My c++ is a little rough
 */
class SetPoint {
  public:
    void set_values(String day_type_, int h_, int m_, int t_, String period_) {
      day_type = day_type_;
      h = h_;
      m = m_;
      temp = t_; 
      period = period_;
      set = true;
    }
    String get_day_type() { return day_type; }
    int get_hour() { return h; }
    int get_min()  { return m; }
    int get_temp() { return temp; }
    void clear_data() { set = false; }
    String get_period() { return period; }
    bool is_set() { return set; }

  private:
    String day_type;
    int h;
    int m;
    String period;
    int temp;
    bool set;
};

// general vars
int current_page = HOME_PAGE;
const char* dayNames[7] = {"Sun", "Mon", "Tues", "Wed", "Thu", "Fri", "Sat"};
const char* am_pm[2] = {"AM", "PM"};
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_FT6206 ts = Adafruit_FT6206();
RTC_DS1307 rtc;
DateTime Clock;
int SetPoints [24] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

// home page vars
int real_temp = 75;
int prev_real_temp = real_temp;
int set_temp = 72;
int prev_set_temp = set_temp;
bool currently_touched = false;
bool auto_on = false;
bool hold_on = false;
int mode = COOL_MODE;
const char* modes[2] = {"Cool Mode", "Heat Mode"};
int temp_pin = 3;
double tempF;
int reading;
int real_time_min;

// time setting page vars
bool am_selected = false;
int current_day = 0;
int previous_day = current_day;
int current_hour = 6;
int previous_hour = current_hour;
int current_minute = 12;
int previous_minute = current_minute;

//set point page vars
int current_set_point = 3;
int current_day_type = WEEKDAY;
const char* day_type[2] = {"Weekday", "Weekend"};
SetPoint weekday_set_points[4];
SetPoint weekend_set_points[4];

// EDIT_SET_POINT_PAGE vars
int curr_edit_hour;
int prev_edit_hour;
int curr_edit_min;
int prev_edit_min;
int curr_edit_temp;
int prev_edit_temp;
int am_edit_selected = false;


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

  // rtc stuff
    if (! rtc.isrunning()) 
      {
        Serial.println("RTC is NOT running!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      }

  //---------sandbox space-----------
  //write weekday set point 1, EEPROM indicies 0,1,2
  EEPROM.write(0, 255);
  EEPROM.write(0, 255);
  EEPROM.write(1, 255);
//  EEPROM.write(2, 255);
//  EEPROM.write(3, 255);
//  EEPROM.write(4, 255);
//  EEPROM.write(5, 255);
//  EEPROM.write(6, 255);
//  EEPROM.write(7, 255);
//  EEPROM.write(8, 255);
//  EEPROM.write(9, 255);
//  EEPROM.write(10, 255);
//  EEPROM.write(11, 255);
  
  current_page = SET_POINTS_PAGE;
  drawSetPointsPage();
  //---------------------------------
//  drawHomeScreen();
}

void loop() {
  Clock = rtc.now();
  if(real_time_min != Clock.minute() && current_page == HOME_PAGE) {
    real_time_min = Clock.minute();
    printDOWandTime(); 
  }
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
      p.x = map(p.x, 0, 240, 240, 0);
      p.y = map(p.y, 0, 320, 320, 0);

      switch(current_page) {
        case HOME_PAGE:
          // settings button
          if(p.x >= BOXSIZE * 0 && p.x <= BOXSIZE * 4 && p.y >= BOXSIZE * 22 && p.y <= BOXSIZE * 32) {
            current_page = SETTINGS_PAGE;
            clearScreen();
            drawSettingsScreen();
          }

          // auto button
          else if(p.x >= BOXSIZE * 20 && p.x <= BOXSIZE * 24 && p.y >= BOXSIZE * 10.5 && p.y <= BOXSIZE * 20.5) {
            auto_on = !auto_on;
            drawBottomBar();
          }

          // hold button
          else if(p.x >= BOXSIZE * 20 && p.x <= BOXSIZE * 24 && p.y >= BOXSIZE * 0 && p.y <= BOXSIZE * 10) {
            hold_on = !hold_on;
            drawBottomBar();
          }

          // up arrow
          else if (p.x >= BOXSIZE * 9 && p.x <= BOXSIZE * 11 && p.y >= BOXSIZE * 2.5 && p.y <= BOXSIZE * 5.5) {
            if (!(set_temp > 89)) {
              prev_set_temp = set_temp;
              set_temp++;
              printSetTemp();
            }
          }

          // down arrow
          else if (p.x >= BOXSIZE * 17 && p.x <= BOXSIZE * 19 && p.y >= BOXSIZE * 2.5 && p.y <= BOXSIZE * 5.5) {
            if (!(set_temp < 41)) {
              prev_set_temp = set_temp;
              set_temp--;
              printSetTemp();
            }
          }
          break;
          
          
        case SETTINGS_PAGE:
          // Go Back
          if(p.x >= BOXSIZE * 0 && p.x <= BOXSIZE * 5 && p.y >= BOXSIZE * 22 && p.y <= BOXSIZE * 32) {
            current_page = HOME_PAGE;
            clearScreen();
            drawHomeScreen();
          }

          // Day & Time
          else if(p.x >= BOXSIZE * 7 && p.x <= BOXSIZE * 12 && p.y >= BOXSIZE * 10 && p.y <= BOXSIZE * 25) {
            current_page = TIME_SETTINGS_PAGE;
            clearScreen();
            drawTimeSettingPage();
          }

          // Set Points
          else if(p.x >= BOXSIZE * 15 && p.x <= BOXSIZE * 20 && p.y >= BOXSIZE * 10 && p.y <= BOXSIZE * 25) {
            current_page = SET_POINTS_PAGE;
            clearScreen();
            drawSetPointsPage();
          }
          break;
          
          
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
          }

          // day down arrow
          else if(p.x >= BOXSIZE * 14 && p.x <= BOXSIZE * 16 && p.y >= BOXSIZE * 23 && p.y <= BOXSIZE * 26) {
            previous_day = current_day;
            if(current_day >= 1) {
              current_day--;
            }
            else {
              current_day = 6;
            }
            // update day label
            updateText(BOXSIZE * 3, BOXSIZE * 11.5, 2, String(dayNames[previous_day]), String(dayNames[current_day]));
          }

          // hour up arrow
          else if(p.x >= BOXSIZE * 6 && p.x <= BOXSIZE * 8 && p.y >= BOXSIZE * 13 && p.y <= BOXSIZE * 16) {
            if(current_hour < 12) {
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
          }

          // hour down arrow
          else if(p.x >= BOXSIZE * 14 && p.x <= BOXSIZE * 16 && p.y >= BOXSIZE * 13 && p.y <= BOXSIZE * 16) {
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
          }  

          // minute up arrow
          else if(p.x >= BOXSIZE * 6 && p.x <= BOXSIZE * 8 && p.y >= BOXSIZE * 7 && p.y <= BOXSIZE * 10) {
            if(current_minute < 59){
              previous_minute = current_minute;
              current_minute++;
            
              if(current_minute > 9 && previous_minute != 9) {
                updateText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String(previous_minute), String(current_minute));
              }
              // special case going from 9 to 10
              else if(current_minute > 9 && previous_minute == 9){
                updateText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String ("0") + String(previous_minute), String(current_minute));
              }
              else {
                updateText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String ("0") + String(previous_minute), String ("0") + String(current_minute));
              }
            }
          }

           // minute down arrow
           else if(p.x >= BOXSIZE * 14 && p.x <= BOXSIZE * 16 && p.y >= BOXSIZE * 7 && p.y <= BOXSIZE * 10) {
             if(current_minute > 0) {
               previous_minute = current_minute;
               current_minute--;
             
               if(current_minute > 9 && previous_minute != 10) {
                 updateText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String(previous_minute), String(current_minute));
               }
               // special case going from 10 to 9
               else if(current_minute == 9 && previous_minute == 10){
                 updateText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String(previous_minute), String ("0") + String(current_minute));
               }
               else {
                 updateText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String ("0") + String(previous_minute), String ("0") + String(current_minute));
               }
             }
           }

           // am
           else if(p.x >= BOXSIZE * 7.5 && p.x <= BOXSIZE * 9.5 && p.y >= BOXSIZE * 1.5 && p.y <= BOXSIZE * 5.5) {
             am_selected = true;
             tft.fillRect(270, 90, 27, 2, ILI9341_WHITE);   // am line on
             tft.fillRect(270, 140, 27, 2, ILI9341_BLACK);  // pm line off
           }

           // pm
           else if(p.x >= BOXSIZE * 12.5 && p.x <= BOXSIZE * 14.5 && p.y >= BOXSIZE * 1.5 && p.y <= BOXSIZE * 5.5) {
             am_selected = false;
             tft.fillRect(270, 90, 27, 2, ILI9341_BLACK);   // am line off
             tft.fillRect(270, 140, 27, 2, ILI9341_WHITE);  // pm line on
           }

           // save
           // TODO: move down
           else if(p.x >= BOXSIZE * 19 && p.x <= BOXSIZE * 22.5 && p.y >= BOXSIZE * 19 && p.y <= BOXSIZE * 27.5) {
             int hr = current_hour;
             // set the RTC
             if(am_selected && hr == 12) {
              //rtc.adjust(DateTime(2014, 1, 21, 0, current_minute, 0));
              hr = 0;
             }
             else if(!am_selected && hr == 12) {
              //rtc.adjust(DateTime(2014, 1, 21, 12, current_minute, 0));
              hr = 12;
             }
             else if(!am_selected) {
              hr += 12;
             }
             rtc.adjust(DateTime(2014, 1, 21, hr, current_minute, 0));
             current_page = SETTINGS_PAGE;
             clearScreen();
             drawSettingsScreen();
           }

           //cancel
           // TODO: move down
           else if(p.x >= BOXSIZE * 19 && p.x <= BOXSIZE * 22.5 && p.y >= BOXSIZE * 7 && p.y <= BOXSIZE * 15.5) {
             current_page = SETTINGS_PAGE;
             clearScreen();
             drawSettingsScreen();
           }
           break;

        case SET_POINTS_PAGE:
          // Go Back
          if(p.x >= BOXSIZE * 0 && p.x <= BOXSIZE * 5 && p.y >= BOXSIZE * 22 && p.y <= BOXSIZE * 32) {
            current_page = SETTINGS_PAGE;
            clearScreen();
            drawSettingsScreen();
          }
          
          // up arrow day type
          else if(p.x >= BOXSIZE * 8.5 && p.x <= BOXSIZE * 10.5 && p.y >= BOXSIZE * 25 && p.y <= BOXSIZE * 28) {
            int previous_day_type;
            if (current_day_type == WEEKDAY) {
              previous_day_type = current_day_type;
              current_day_type = WEEKEND;
              }
            else {
              previous_day_type = current_day_type;
              current_day_type = WEEKDAY;
            }
            updateText(BOXSIZE * 2, BOXSIZE * 13.5, 1, day_type[previous_day_type], day_type[current_day_type]);
        
            drawSetPointsPage();
          }
          
          // down arrow day type
          else if(p.x >= BOXSIZE * 17.5 && p.x <= BOXSIZE * 19.5 && p.y >= BOXSIZE * 25 && p.y <= BOXSIZE * 28) {
              int previous_day_type;
              if (current_day_type == WEEKDAY) {
                previous_day_type = current_day_type;
                current_day_type = WEEKEND;
                }
              else {
                previous_day_type = current_day_type;
                current_day_type = WEEKDAY;
            }
            updateText(BOXSIZE * 2, BOXSIZE * 13.5, 1, day_type[previous_day_type], day_type[current_day_type]);
            
          }
          
          // set point 1
          else if(p.x >= BOXSIZE * 5 && p.x <= BOXSIZE * 8.5 && p.y >= BOXSIZE * 2 && p.y <= BOXSIZE * 19) {
            current_set_point = 0;
            current_page = EDIT_SET_POINT_PAGE;

            // set EDIT_SET_POINT_PAGE vars
            // TODO: try not to repeat this code 3 more times
            if (EEPROM.read(current_set_point*3) != NOT_SET) {
              curr_edit_hour = EEPROM.read(current_set_point*3);
              if(curr_edit_hour > 12) {
                curr_edit_hour -= 12;
              }
              curr_edit_min = EEPROM.read(current_set_point*3+1);
              curr_edit_temp = EEPROM.read(current_set_point*3+2);

            }
            prev_edit_hour = curr_edit_hour;
            prev_edit_min = curr_edit_min;
            prev_edit_temp = curr_edit_temp;
            clearScreen();
            drawEditSetPointPage();
          }

          // set point 2
          else if(p.x >= BOXSIZE * 10 && p.x <= BOXSIZE * 13.5 && p.y >= BOXSIZE * 2 && p.y <= BOXSIZE * 19) {
            current_set_point = 1;
            current_page = EDIT_SET_POINT_PAGE;
            clearScreen();
            drawEditSetPointPage();
          }

          // set point 3
          else if(p.x >= BOXSIZE * 15 && p.x <= BOXSIZE * 18.5 && p.y >= BOXSIZE * 2 && p.y <= BOXSIZE * 19) {
            current_set_point = 2;
            current_page = EDIT_SET_POINT_PAGE;
            clearScreen();
            drawEditSetPointPage();
          }

          // set point 4
          else if(p.x >= BOXSIZE * 20 && p.x <= BOXSIZE * 23.5 && p.y >= BOXSIZE * 2 && p.y <= BOXSIZE * 19) {
            current_set_point = 3;
            current_page = EDIT_SET_POINT_PAGE;
            clearScreen();
            drawEditSetPointPage();
          }
          break;
        
         
        case EDIT_SET_POINT_PAGE:
          // day up arrow
          if(p.x >= BOXSIZE * 6 && p.x <= BOXSIZE * 8 && p.y >= BOXSIZE * 23 && p.y <= BOXSIZE * 26) {
            if (!(curr_edit_temp > 89)) {
              prev_edit_temp = curr_edit_temp;
              curr_edit_temp++;
              updateText(BOXSIZE * 4.5, BOXSIZE * 11.5, 2, String(prev_edit_temp), String(curr_edit_temp));
            }
          }

          // day down arrow
          else if(p.x >= BOXSIZE * 14 && p.x <= BOXSIZE * 16 && p.y >= BOXSIZE * 23 && p.y <= BOXSIZE * 26) {
            if (!(curr_edit_temp < 41)) {
              prev_edit_temp = curr_edit_temp;
              curr_edit_temp--;
              updateText(BOXSIZE * 4.5, BOXSIZE * 11.5, 2, String(prev_edit_temp), String(curr_edit_temp));
            }
          }

          // hour up arrow
          else if(p.x >= BOXSIZE * 6 && p.x <= BOXSIZE * 8 && p.y >= BOXSIZE * 13 && p.y <= BOXSIZE * 16) {
            if(curr_edit_hour < 12) {
              prev_edit_hour = curr_edit_hour;
              curr_edit_hour++;
              float ref_x;
              if(curr_edit_hour > 9 && prev_edit_hour != 9) {
                ref_x = 15;
              }
              // special case when going from 9 to 10
              else if(curr_edit_hour > 9 && prev_edit_hour == 9){
                  updateText(BOXSIZE * 16.5, BOXSIZE * 11.5, BOXSIZE * 15, 2, String(prev_edit_hour), String(curr_edit_hour));
                  break;
              }
              else {
                ref_x = 16.5;
              }
              updateText(BOXSIZE * ref_x, BOXSIZE * 11.5, 2, String(prev_edit_hour), String(curr_edit_hour));
            }
          }

          // hour down arrow
          else if(p.x >= BOXSIZE * 14 && p.x <= BOXSIZE * 16 && p.y >= BOXSIZE * 13 && p.y <= BOXSIZE * 16) {
            if (curr_edit_hour > 1) {
              prev_edit_hour = curr_edit_hour;
              curr_edit_hour--;
            
              float ref_x;
              if(curr_edit_hour < 10 && prev_edit_hour != 10) {
                ref_x = 16.5;
              }
              // special case when going from 10 to 9
              else if(curr_edit_hour < 10 && prev_edit_hour == 10) {
                updateText(BOXSIZE * 15, BOXSIZE * 11.5, BOXSIZE * 16.5, 2, String(prev_edit_hour), String(curr_edit_hour));
                break;
              }
              else {
                ref_x = 15;
              }
              updateText(BOXSIZE * ref_x, BOXSIZE * 11.5, 2, String(prev_edit_hour), String(curr_edit_hour));
            }
          }

          // minute up arrow
          else if(p.x >= BOXSIZE * 6 && p.x <= BOXSIZE * 8 && p.y >= BOXSIZE * 7 && p.y <= BOXSIZE * 10) {
            if(curr_edit_min < 59){
              prev_edit_min = curr_edit_min;
              curr_edit_min++;
            
              if(curr_edit_min > 9 && prev_edit_min != 9) {
                updateText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String(prev_edit_min), String(curr_edit_min));
              }
              // special case going from 9 to 10
              else if(curr_edit_min > 9 && prev_edit_min == 9){
                updateText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String ("0") + String(prev_edit_min), String(curr_edit_min));
              }
              else {
                updateText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String ("0") + String(prev_edit_min), String ("0") + String(curr_edit_min));
              }
            }
          }

           // minute down arrow
           else if(p.x >= BOXSIZE * 14 && p.x <= BOXSIZE * 16 && p.y >= BOXSIZE * 7 && p.y <= BOXSIZE * 10) {
             if(curr_edit_min > 0) {
               prev_edit_min = curr_edit_min;
               curr_edit_min--;
             
               if(curr_edit_min > 9 && prev_edit_min != 10) {
                 updateText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String(prev_edit_min), String(curr_edit_min));
               }
               // special case going from 10 to 9
               else if(curr_edit_min == 9 && prev_edit_min == 10){
                 updateText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String(prev_edit_min), String ("0") + String(curr_edit_min));
               }
               else {
                 updateText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String ("0") + String(prev_edit_min), String ("0") + String(curr_edit_min));
               }
             }
           }

          // am
           else if(p.x >= BOXSIZE * 7.5 && p.x <= BOXSIZE * 9.5 && p.y >= BOXSIZE * 1.5 && p.y <= BOXSIZE * 5.5) {
             am_edit_selected = true;
             tft.fillRect(270, 90, 27, 2, ILI9341_WHITE);   // am line on
             tft.fillRect(270, 140, 27, 2, ILI9341_BLACK);  // pm line off
           }

           // pm
           else if(p.x >= BOXSIZE * 12.5 && p.x <= BOXSIZE * 14.5 && p.y >= BOXSIZE * 1.5 && p.y <= BOXSIZE * 5.5) {
             am_edit_selected = false;
             tft.fillRect(270, 90, 27, 2, ILI9341_BLACK);   // am line off
             tft.fillRect(270, 140, 27, 2, ILI9341_WHITE);  // pm line on
           }
          
         // save
         // TODO: move down
         if(p.x >= BOXSIZE * 19 && p.x <= BOXSIZE * 22.5 && p.y >= BOXSIZE * 19 && p.y <= BOXSIZE * 27.5) {
           if(current_day_type == WEEKDAY) {
             // put into eeprom
             if(am_edit_selected && curr_edit_hour == 12) {
              curr_edit_hour = 0;
             }
             else if(!am_edit_selected && curr_edit_hour == 12) {
              curr_edit_hour = 12;
             }
             else if(!am_edit_selected) {
              curr_edit_hour += 12;
             }
             EEPROM.write(current_set_point*3, curr_edit_hour);
             EEPROM.write(current_set_point*3+1, curr_edit_min);
             EEPROM.write(current_set_point*3+2, curr_edit_temp);
           }
           else {
             EEPROM.write(current_set_point*3+12, curr_edit_hour);
             EEPROM.write(current_set_point*3+12+1, curr_edit_min);
             EEPROM.write(current_set_point*3+12+2, curr_edit_temp);
           }
           current_page = SET_POINTS_PAGE;
           clearScreen();
           drawSetPointsPage();
         }

         //cancel
         // TODO: move down
         if(p.x >= BOXSIZE * 19 && p.x <= BOXSIZE * 22.5 && p.y >= BOXSIZE * 7 && p.y <= BOXSIZE * 15.5) {
          int index_offset = 0;
          if(current_day_type == WEEKEND) { index_offset = 12; }
          if(EEPROM.read(current_set_point*3+index_offset) != NOT_SET)
          {
            // clear eeprom
            EEPROM.write(current_set_point*3+index_offset, 255);
            EEPROM.write(current_set_point*3+1+index_offset, 255);
            EEPROM.write(current_set_point*3+2+index_offset, 255);
          }
           current_page = SET_POINTS_PAGE;
           clearScreen();
           drawSetPointsPage();
         }
         break;
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
 *  - EDIT_SET_POINT_PAGE
 */

/**
 * Draws Home page
 */
// TODO: could probably refactor to be more like settings page with more printText calls
void drawHomeScreen() {
  // TODO: do i need this?
//  printText(BOXSIZE * 22, BOXSIZE * 11, 1, "Set", ILI9341_WHITE);
//  printText(BOXSIZE * 22.5, BOXSIZE * 13, 1, "to", ILI9341_WHITE);
  drawCornerButton("Settings");
  printSetTemp();
  drawArrows(BOXSIZE * 26.5, BOXSIZE * 9, ARROW_WIDTH, ARROW_HEIGHT, ARROWS_OFFSET, ILI9341_WHITE);
  printDOWandTime();
  printCurrentTemp();
  drawBottomBar();
}

/**
 * Draws Settings Screen
 */
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

/**
 * Draws Time Settings Page
 */
void drawTimeSettingPage() {
  // Day label
  printText(BOXSIZE * 5, BOXSIZE * 2, 1, "Day", ILI9341_WHITE);

  // Time Label
  printText(BOXSIZE * 18.5, BOXSIZE * 2.5, 1, "Time", ILI9341_WHITE);

  // toggling arrows
  drawArrows(BOXSIZE * 5, BOXSIZE * 7.5, ARROW_WIDTH, ARROW_HEIGHT, ARROWS_OFFSET, ILI9341_WHITE);   // day
  drawArrows(BOXSIZE * 16, BOXSIZE * 7.5, ARROW_WIDTH, ARROW_HEIGHT, ARROWS_OFFSET, ILI9341_WHITE);  // hour
  drawArrows(BOXSIZE * 22, BOXSIZE * 7.5, ARROW_WIDTH, ARROW_HEIGHT, ARROWS_OFFSET, ILI9341_WHITE);  // minute

  // day
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

/**
 * Draws Set Points Page
 */
void drawSetPointsPage() {
  int offset = 5;
  String set_point_text;
  SetPoint sp[4];
  
  drawCornerButton("Go Back");
  printText(BOXSIZE * 15, BOXSIZE *  2.5, 1, "Your Set Points", ILI9341_WHITE);  // title

  // week/weekend toggling
  drawArrows(BOXSIZE * 3.5, BOXSIZE * 10, 30, 20, 60, ILI9341_WHITE);
  printText(BOXSIZE * 2, BOXSIZE * 13.5, 1, day_type[current_day_type], ILI9341_WHITE);

//  // get correct set point array
//  for(int i=0; i<4; i++) {
//    if(current_day_type == WEEKDAY) {
//      sp[i] = weekday_set_points[i];
//    }
//    else {
//      sp[i] = weekend_set_points[i];
//    }
//  }
//  
  int index_offset = 0;
  if (current_day_type == WEEKEND) { index_offset = 12; }

  // prints the 4 set point boxes with their data
  for(int i=0; i<4; i++) {
    tft.drawRect(BOXSIZE * 13, BOXSIZE * (i*offset+5), BOXSIZE * 17, BOXSIZE * 3.5, ILI9341_WHITE); // box
      
    printText(BOXSIZE * 14, BOXSIZE * (i*offset+5)+20, 1, String(i+1), ILI9341_WHITE);
    tft.drawLine(BOXSIZE * 16, BOXSIZE * (i*offset+5), BOXSIZE * 16, BOXSIZE * (i*offset+8.4), ILI9341_WHITE);

    // get data from eeprom
    String set_point_text;
    if(EEPROM.read(i*3+index_offset) != 255) {
      int hr = EEPROM.read(i*3+index_offset);
      int mn = EEPROM.read(i*3+1+index_offset);
      String temp = String(EEPROM.read(i*3+2+index_offset));
      String mn_ = String(mn);
      String period = "AM";
      if(hr == 0) {
        hr = 12;
      }
      else if(hr == 12) {
        period = "PM";
      }
      else if(hr > 12) {
        hr -= 12;
        period = "PM";
      }
      if(mn < 10) {
        mn_ = "0" + String(mn);
      }
      set_point_text = String(hr) + ":" + mn_ + " " + period + "  " + temp + " F";
    }
    else {
      set_point_text = "Not Set";
    }
    tft.fillRect(BOXSIZE * 16.5, BOXSIZE * (i*offset+5.5), BOXSIZE * 13, BOXSIZE * 2.5, ILI9341_BLACK);
    printText(BOXSIZE * 17, BOXSIZE * (i*offset+5)+20, 1, set_point_text, ILI9341_WHITE);
  }    
}

/**
 * Edit individual set point page
 */
void drawEditSetPointPage() {
  String day_type;
  int i;
  SetPoint sp;
  if(current_day_type == WEEKDAY) {
    i = current_set_point*3;
    day_type = "Weekday";
  }
  else {
    i = current_set_point*3+12;
    day_type = "Weekend";
  }


  int hr = 0;
  int mn = 0;
  int temp = 0;  
  if(EEPROM.read(i) == 255)
  {
     hr = 12;
     mn = 0;
     temp = 60;
     curr_edit_hour = hr;
     curr_edit_min = mn;
     curr_edit_temp = temp;
     
  }
  else
  {
     hr = EEPROM.read(i);
     mn = EEPROM.read(i+1);
     temp = EEPROM.read(i+2);  
  }

  // scale hr
  if(hr > 12) {
    hr -= 12;
  }
  else if (hr == 0) {
    hr = 12;
  }
  
  //title
  printText(BOXSIZE * 7, BOXSIZE *  2.5, 1, day_type + " Set Point " + String(current_set_point+1), ILI9341_WHITE);
  
  // toggling arrows
  drawArrows(BOXSIZE * 5, BOXSIZE * 7.5, ARROW_WIDTH, ARROW_HEIGHT, ARROWS_OFFSET, ILI9341_WHITE);   // temp
  drawArrows(BOXSIZE * 16, BOXSIZE * 7.5, ARROW_WIDTH, ARROW_HEIGHT, ARROWS_OFFSET, ILI9341_WHITE);  // hour
  drawArrows(BOXSIZE * 22, BOXSIZE * 7.5, ARROW_WIDTH, ARROW_HEIGHT, ARROWS_OFFSET, ILI9341_WHITE);  // minute

  // temp
  printText(BOXSIZE * 4.5, BOXSIZE * 11.5, 2, String(temp), ILI9341_WHITE);

  // hour
  if(hr > 9) {
    printText(BOXSIZE * 15, BOXSIZE * 11.5, 2, String(hr), ILI9341_WHITE);
  }
  else {
    printText(BOXSIZE * 16.5, BOXSIZE * 11.5, 2, String(hr), ILI9341_WHITE);
  }
  
  // minute
  if(mn > 9) {
    printText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String(mn), ILI9341_WHITE);
  }
  else {
    printText(BOXSIZE * 21.5, BOXSIZE * 11.5, 2, String ("0") + String(mn), ILI9341_WHITE);
  }

  // time colon
  tft.fillRect(203, 95, 4, 4, ILI9341_WHITE);
  tft.fillRect(203, 110, 4, 4, ILI9341_WHITE);

  // am vs pm
  printText(BOXSIZE * 27, BOXSIZE * 8.5, 1, "AM", ILI9341_WHITE);
  printText(BOXSIZE * 27, BOXSIZE * 13.5, 1, "PM", ILI9341_WHITE);

  // selected line
  if (am_edit_selected) {
    tft.fillRect(270, 90, 27, 2, ILI9341_WHITE);
  }
  else {
    tft.fillRect(270, 140, 27, 2, ILI9341_WHITE);
  }

  // save button
  tft.drawRect(45, 190, 85, 35, ILI9341_WHITE);
  printText(68, 213, 1, "Save", ILI9341_WHITE);
  
  // cancel button
  String label = "Delete";
  if(EEPROM.read(i) == 255)
  {
    label = "Cancel";
  }
  tft.drawRect(165, 190, 85, 35, ILI9341_WHITE);
  printText(178, 213, 1, label, ILI9341_WHITE);
}

/**
 * General helper methods
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

// overloaded version for Bold fonts
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

// draws an arrow
void drawArrow(int x_start, int y_start, int x_width, int y_height, uint16_t color) {
  tft.fillTriangle(x_start, y_start, x_start+(x_width/2), y_start+y_height, x_start+x_width, y_start, color);
}

// draws two identical arrows: one up, one down
void drawArrows(int x_start, int y_start, int x_width, int y_height, int y_offset, uint16_t color ) {
//  int y_offset = 60;
  // up
  drawArrow(x_start, y_start, x_width, -y_height, ILI9341_WHITE);

  // down
  drawArrow(x_start, y_start+y_offset, x_width, y_height, ILI9341_WHITE);
}

// modular corner button 
void drawCornerButton(String label) {
  tft.drawLine(BOXSIZE * 0, BOXSIZE * 4, BOXSIZE * 10, BOXSIZE * 4, ILI9341_WHITE);
  tft.drawLine(BOXSIZE * 10, BOXSIZE * 4, BOXSIZE * 10, BOXSIZE * 0, ILI9341_WHITE);
  printText(BOXSIZE * 1.5, BOXSIZE * 2, 1, label, ILI9341_WHITE);
}

/**
 * These helper methods draw home screen items
 */
// contains mode, auto status, hold status
void drawBottomBar() {
  // horiz line
  tft.drawLine(0, 200, 320, 200, ILI9341_WHITE);

  // Cool/Heat status
//  printMode();
  printText(BOXSIZE, BOXSIZE * 22.5, 1, modes[mode], ILI9341_WHITE);

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

// future work
//void printMode() {
//  if (set_temp > real_temp) { //cool or off
//    if(mode == COOL_MODE || mode == AUTO_MODE) {
//      // print cooling
//    }
//    else {
//      
//    }
//  }
//  printText(BOXSIZE, BOXSIZE * 22.5, 1, modes[mode], ILI9341_WHITE);
//}

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

// Prints the Day of Week and current time
void printDOWandTime(){
  //clear time text
  tft.fillRect(BOXSIZE * 19, BOXSIZE * 0, BOXSIZE * 11, BOXSIZE * 2, ILI9341_BLACK);
  
  Clock = rtc.now();
  int hr = Clock.hour();
  int mn = Clock.minute();
  String mn_ = String(mn);
  String period = "AM";
  if(hr == 0) {
    hr = 12;
  }
  else if(hr == 12) {
    period = "PM";
  }
  else if(Clock.hour() > 12) {
    hr = Clock.hour() - 12;
    period = "PM";
  }

  if(mn < 10) {
    mn_ = "0" + String(mn);
  }
  
  String str = "Fri " + String(hr) + ":" + mn_ + " " + period;
  printText(BOXSIZE * 20, BOXSIZE * 1.5, 1, str, ILI9341_WHITE);
}

// Prints the set temperature
void printSetTemp() {
  updateText(BOXSIZE * 26, BOXSIZE * 13, 2, String(prev_set_temp), String(set_temp));
}

void getTemp() {
  reading = analogRead(temp_pin);
  tempF = (reading / 9.31 * 1.8 + 32);
}
