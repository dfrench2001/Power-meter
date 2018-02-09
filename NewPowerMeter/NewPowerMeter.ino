// Libraries
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_INA219.h>
#include <ESPHelper.h>
#include <AdafruitIO_WiFi.h>
#include <Metro.h> // Include the Metro library

//#define SERIAL_DEBUG
//#define DEBUG

#define SSID            "2POTATO"
#define NETWORK_PASS    "2micamarleykato"

#define AIO_USERNAME    "esp8266forDale"
#define AIO_KEY         "409319d05faf425a9c86de804e423a4f"
#define AIO_FEED        "power"
#define AIO_FEED2       "current"

AdafruitIO_WiFi io(AIO_USERNAME, AIO_KEY, SSID, NETWORK_PASS);

// set up the feeds to AdafruitIO
AdafruitIO_Feed *power_feed = io.feed("power");
AdafruitIO_Feed *current_feed = io.feed("current");
AdafruitIO_Feed *Battery_level_feed = io.feed("Battery level");
AdafruitIO_Feed *counter_feed = io.feed("counter");
AdafruitIO_Feed *LED_feed = io.feed("LED");

ADC_MODE(ADC_VCC);

netInfo homeNet = {	.mqttHost = "io.adafruit.com",	 //can be blank if not using MQTT
		            		.mqttUser = AIO_USERNAME, 	     //can be blank
      	           	.mqttPass = AIO_KEY,             //can be blank
		            		.mqttPort = 1883,		             //default port for MQTT is 1883 - only chance if needed.
	            			.ssid = "2POTATO",
		            		.pass = "2micamarleykato"};

ESPHelper myESP(&homeNet);

// INA sensor
Adafruit_INA219 ina219;

// OLED screen
#define OLED_RESET 3
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Measurement variables
float current_mA;
float power_mW;
float battery_V;

// Time to sleep (in seconds):
const int sleepTimeS = 10;
const int sleepTimeMin = 60;
const int sleepTimeHour = sleepTimeMin * 60 ;

Metro feedMetro = Metro(sleepTimeMin * 10);  // 10 min timer to update AdafruitIO feeds
Metro sleepMetro = Metro(sleepTimeMin * 5); // 5 min timer to go in sleep mode

unsigned long lastMillis = 0;

void setup()   {

  Serial.begin(115200);

  //  Setup OVER THE AIR (wifi) sketch updates
  OTA_Setup();

  // connect to io.adafruit.com
  io.connect();

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

  //setup ESPHelper
  myESP.begin();

  // Init INA219
  ina219.begin();
  ina219.setCalibration_16V_400mA();

  // Init OLED Display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

   // Display welcome message
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,16);
  display.println("Ready!");
  display.display();
  delay(2000);

  // LED
  pinMode(12, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
 
 // Turn off BUILT_IN LED. Turning it on from AdafruitIO, enabbles 5 on 30 minute off power saving mode
  digitalWrite(LED_BUILTIN,HIGH);   // Turn off BUILT_IN LED; 
 

}
void loop() {

   myESP.loop();  //run the loop() method as often as possible - this keeps the network services running
   io.run();

   lastMillis = millis() /1000;
     
  // External LED off
  digitalWrite(12, HIGH);

  // setup handler for LED actions
  LED_feed->onMessage(handleLED);

  // Measure
  current_mA = measureCurrent();
  power_mW = measurePower();

  // Display data
  displayData( current_mA, power_mW );

      
//  display.clearDisplay();
//  display.setTextSize(2);
//  display.setTextColor(WHITE);
//  display.setCursor(0,16);
//  display.println("Ready!");
//  display.display();
//  delay(1000);

    delay(3000);

  // if 5 minutes have elasped, go to sleep for 30 minutes
    if (( sleepMetro.check() == 1 ) && (digitalRead(LED_BUILTIN) == LOW))
    {

        display.clearDisplay();
        display.display();
        delay(100);
        Serial.println("Going to sleep----------------------");
  
        ESP.deepSleep((sleepTimeMin * 30) * 1000000, WAKE_RF_DEFAULT);
        delay(100); // wait for deep sleep to happen ...
    }

   battery_V = ESP.getVcc();

  // save to the feed on Adafruit IO
  if(feedMetro.check() == 1)
  {
    current_feed->save(current_mA);
    power_feed->save(power_mW);
    Battery_level_feed->save(battery_V / 1000);    
    counter_feed->save(lastMillis );
  }


  yield();
}
// Function to measure current
float measureCurrent() {

  float shuntvoltage =0;
  float busvoltage = 0;
  float current = 0; // Measure in milli amps
  float loadvoltage = 0;
  // Measure
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

#ifdef DEBUG
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.println("");
#endif

  // If negative, set to zero
//  if (current_mA < 0) {
//    current_mA = 0.0;
//  }

  return current_mA;

}

// Function to measure power
float measurePower() {
  float shuntvoltage =0;
  float busvoltage = 0;
  float current = 0; // Measure in milli amps
  float loadvoltage = 0;

  // Measure
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

#ifdef DEBUG
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(current_mA * loadvoltage); Serial.println(" mW");
  Serial.println("");
#endif

  // If negative, set to zero
//  if (current_mA < 0) {
//    current_mA = 0.0;
//  }

  return current_mA * loadvoltage;

}

// Display measurement data
void displayData(float current, float power) {

  // Clear
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  // Power
  display.setCursor(0,0);
  display.println("Power: ");
  display.print(power);
  display.println(" mW");

  // Displays
  display.display();

  delay(2000);

  // Current
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Current: ");
  display.print(current);
  display.println(" mA");

  // Displays
  display.display();

}

void OTA_Setup()
{
// Setup Over the Air uptades
  myESP.OTA_enable();
	myESP.OTA_setPassword("jasons esp");
	myESP.OTA_setHostnameWithVersion("CayenneVersion .9");

	myESP.begin();

}
void callback(char* topic, uint8_t* payload, unsigned int length) {
	//put mqtt callback code here
}

void handleLED(AdafruitIO_Data *data)
{
  
  if(data->toString() == "ON")
  {
    Serial.println( " ON SENT");
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  { Serial.println( " OFF SENT");
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
