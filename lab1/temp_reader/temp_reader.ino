//#include <OneWire.h> 
//#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2 
//OneWire oneWire(ONE_WIRE_BUS); 
int incomingByte = 0;

//DallasTemperature sensors(&oneWire);
void setup(void) { 
  Serial.begin(9600);
  //sensors.begin();
  pinMode(LED_BUILTIN, OUTPUT); 
} 
void loop(void) {
  if (Serial.available() == 0) {
    blinkLED(200);
  }
  while (Serial.available() > 0) {
    blinkLED(1000);
    incomingByte = Serial.read(); // read the incoming byte:
    Serial.print(" I received:");
    Serial.println(incomingByte);
  }

// Serial.print(" reading temperature..."); 
// //sensors.requestTemperatures();
// Serial.println("DONE"); 
// Serial.print("Temperature is: "); 
// Serial.print(27.5/*sensors.getTempCByIndex(0)*/); 
// delay(1000); 
} 

void blinkLED(int ms) {
  for(int i=0; i<5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(ms);
    digitalWrite(LED_BUILTIN, LOW);
    delay(ms);
  }
}
