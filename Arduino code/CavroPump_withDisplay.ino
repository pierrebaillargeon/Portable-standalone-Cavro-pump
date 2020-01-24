/* Arduino firmware for Cavro pump               */
/* Scripps Florida                               */ 
/* Authors: Pierre Baillargeon                   */
/* Correspondence: bpierre@scripps.edu           */ 
/* Date: 1/24/2020                               */ 


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* How long to wait between aspirate and dispense steps, in milliseconds */ 
int loopDelay = 5000; 
int pumpDelayKnobPin = 0;
int pumpDelayKnobRawValue = 0;
int pumpDelayKnobLastRawValue = 0;
int pumpDelayKnobMappedValue = 0;
int pumpSpeedKnobPin = 1;
int pumpSpeedKnobRawValue = 0;
int pumpSpeedKnobLastRawValue = 0;
int pumpSpeedKnobMappedValue = 0;
int pumpSpeedPercentageValue=0;
float tempValue=0.00;
char displayPumpDelayValue[16];
String setSpeedCommand="";
int pumpPosition=0;
unsigned long delayStart=0;

void setup() {
  
  // start serial port at 9600 bps:
  Serial.begin(9600);

 // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();

  /* Wait 10 seconds to wait for the pump to power on before attempting to send initialziation command */ 
  delay(10000);
  
  Serial.write("\r");  

  /* Initialize the pump with command /1ZR for port 1 */ 
  Serial.write("/1ZR\r");  
  /* Wait 10 seconds to make sure the pump has finished initializing before sending the next command */ 
  delay(10000);
  delayStart = millis();
  /* Set the speed: valid range is 0 to 40 */ 
  //Serial.println("/1S1R");
  
}

void loop() {

  /* Get updated values from user input/potentiometers */ 
  pumpDelayKnobRawValue = analogRead(pumpDelayKnobPin);
  if(pumpDelayKnobRawValue == 0 || abs(pumpDelayKnobRawValue - pumpDelayKnobLastRawValue) > 20){
    pumpDelayKnobLastRawValue=pumpDelayKnobRawValue;        
  }  

  // was 1000, 10000
  pumpDelayKnobMappedValue = map(pumpDelayKnobLastRawValue,0,1002,10000,1000);  
  
  pumpSpeedKnobRawValue = analogRead(pumpSpeedKnobPin);
  if(pumpSpeedKnobRawValue == 0 || abs(pumpSpeedKnobRawValue - pumpSpeedKnobLastRawValue) > 20){
    pumpSpeedKnobLastRawValue = pumpSpeedKnobRawValue;        
  }
  pumpSpeedKnobMappedValue = map(pumpSpeedKnobLastRawValue,0,1100,5800,500);
  /* prepare OLED for display update */ 
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  
  /* Display speed on OLED */ 
  pumpSpeedPercentageValue=map(pumpSpeedKnobLastRawValue,0,1100,100,0);
  display.print(F("Speed:"));
  display.print(pumpSpeedPercentageValue);
  display.print(F("%"));
  display.setCursor(0,16);

  /* Display delay on OLED */ 
  tempValue = pumpDelayKnobMappedValue / 1000.0;
  dtostrf(tempValue,2,1,displayPumpDelayValue);  
  display.print(F("Delay:"));
  display.print(displayPumpDelayValue);
  display.println(F("s"));
  display.display();
  
  /* Check to see if we have waited for the delay pariod set by the user */ 
  if((millis() - delayStart ) >= pumpDelayKnobMappedValue ) {

    /* reset the timer for the next loop */ 
    delayStart = millis();

    /* generate the Cavro pump set velocity command necessary to send parameter as set by user */ 
    setSpeedCommand = (String)"/1V" + pumpSpeedKnobMappedValue + (String)"R\r"; 
    Serial.print(setSpeedCommand);
    delay(150);

    /* Check to see if the pump is in lower or upper position, send command to move to opposite position */ 
    if(pumpPosition==0) {
      Serial.write("/1A2000R\r"); 
      pumpPosition=1;      
    }
    else {
      Serial.write("/1A0R\r"); 
      pumpPosition=0;
    }
  }
}
