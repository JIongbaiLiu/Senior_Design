#include <OneWire.h> 
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2 
OneWire oneWire(ONE_WIRE_BUS); 


//Pin out
int dataPin = 11; 
int clockPin = 12;
int latchPin = 8;

//Global vars
float tempHist[300] = {-128};
int index = 0;
float currentTemp = 0;
char command = '0';

DallasTemperature sensors(&oneWire);

void setup(void) 
{ 
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);  
  pinMode(latchPin, OUTPUT); 
  updateLEDs(0);
  Serial.begin(38400);
  sensors.begin(); 
  delay(5);
} 


void loop(void) 
{ 
   getSerialInput();
   getNewTemp();
   switch(command){
    case 'g':
      sendHistory();
      break;
    case 'b':
      binDisplay();
      break;
    case 'f':
      updateLEDs(0);
      break;
    default:
      break;
   }
   delay(1000); 
} 
void getSerialInput(){
  if(Serial.available() > 0){
    command = Serial.read();
  }
}
void sendHistory(void){
  for(int i = 0; i < 300; i++){
    Serial.println(tempHist[i]);
    delay(1);
  }
}

void getNewTemp(void){
 sensors.requestTemperatures();
 currentTemp = sensors.getTempCByIndex(0);
 tempHist[index] = currentTemp;
 index++;
 if(index == 300){
  index = 0;
 }
 sendFloat(currentTemp); 
}

void sendFloat(float f){
  byte * b = (byte *) &f;
  Serial.write(b[0]);
  Serial.write(b[1]);
  Serial.write(b[2]);
  Serial.write(b[3]);
}

void binDisplay(){
  int displayTemp = (int)currentTemp;
  updateLEDs(displayTemp);
}
void updateLEDs(int value){
  digitalWrite(latchPin, LOW);     //Pulls the chips latch low
  shiftOut(dataPin, clockPin, MSBFIRST, value); //Shifts out the 8 bits to the shift register
  digitalWrite(latchPin, HIGH);   //Pulls the latch high displaying the data
  delay(500); 
}
