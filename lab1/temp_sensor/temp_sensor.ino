#include <OneWire.h> 
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2 
OneWire oneWire(ONE_WIRE_BUS); 


//Pin out
int dataPin = 11; 
int clockPin = 12;
int latchPin = 8;
int interruptPin = 3;

//Global vars
float tempHist[300];
int index = 0;
float currentTemp = 0;
unsigned long lastTempReadTime = 0;
bool isDisplayOn = false;
bool blockSending = true;

DallasTemperature sensors(&oneWire);

void setup(void) 
{ 
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);  
  pinMode(latchPin, OUTPUT); 
  updateLEDs(0);
  Serial.begin(115200); //make sure this matches what is specified in the python program
  sensors.begin(); 
  lastTempReadTime = millis();
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), buttonEvent, CHANGE);
  for(int i=0; i<300; i++) {
    tempHist[i] = -187;
  }
} 


void loop(void) 
{ 
  
  // if it's been longer than 1 second, read current temp and store it
  if(millis() - lastTempReadTime > 1000) {
    lastTempReadTime = millis();
    getNewTemp();
  }

  // turn on/off LEDs based on display flag
  if(isDisplayOn == true) {
    binDisplay();
  }
  else {
    updateLEDs(0);
  }
} 

void serialEvent() {
  if(Serial.available() > 0) {
    byte incomingByte = Serial.read();
    
    switch(incomingByte) {
      case 0:
        isDisplayOn = false;
        break;
      case 1:
        isDisplayOn = true;
        break;
      case 2:
        sendHistory();
        break;
      default:
        break;
    }
  }
}

void buttonEvent() {
  // toggle display flag
  if(digitalRead(interruptPin) == LOW) {
    isDisplayOn = true;
  }
  else {
    isDisplayOn = false;
  }
}

void sendHistory(){
  float currTempHistory[300];
  for(int i=0; i<300; i++) {
    currTempHistory[i] = tempHist[i];
  }
  for(int i=0; i<300; i++) {
    sendFloat(tempHist[i]);
  }
  blockSending = false;
}

void getNewTemp(){
 sensors.requestTemperatures();
 currentTemp = sensors.getTempCByIndex(0);
 tempHist[index] = currentTemp;
 index++;
 if(index == 299){
  index = 0;
 }
 if(!blockSending) {
  sendFloat(currentTemp);
 }
}

void sendFloat(float f) {
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
