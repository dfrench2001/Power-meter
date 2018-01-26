// Libraries
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_INA219.h>
#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP8266.h>
#include <ESPHelper.h>

//#define SERIAL_DEBUG
#define DEBUG

// WiFi network info.
char ssid[] = "2POTATO";
char wifiPassword[] = "2micamarleykato";

ADC_MODE(ADC_VCC);

netInfo homeNet = {	.mqttHost = "",			//can be blank if not using MQTT
				.mqttUser = "", 	//can be blank
   			.mqttPass = "", 	//can be blank
				.mqttPort = 1883,		//default port for MQTT is 1883 - only chance if needed.
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

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "b6aeb380-fafa-11e7-848a-61efd1c01e7d";
char password[] = "aef8ee1a995aa364532bf32149bfbf4f5cd04f66";
char clientID[] = "7699bbf0-fb0e-11e7-ab60-0b4a23fb76f3";

unsigned long lastMillis = 0;

void setup()   {

  Serial.begin(115200);

  //  Setup OVER THE AIR (wifi) sketch updates
  OTA_Setup();

  // Init INA219
  ina219.begin();
  ina219.setCalibration_16V_400mA();

  // Init OLED Display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // setup router to interfaace with Cayenne Iot website
  Cayenne.begin(username, password, clientID, ssid, wifiPassword);

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
//  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output


}
void loop() {

  Cayenne.loop(); // check devices that send data to Cheyenne
  myESP.loop();  //run the loop() method as often as possible - this keeps the network services running

  // External LED on
  digitalWrite(12, HIGH);

  // Measure
//  current_mA = measureCurrent();
//  power_mW = measurePower();

  // Display data
//  displayData(current_mA, power_mW);
//  displayData(power_mW,current_mA);
//  delay(500);


  // LED 0ff
//  digitalWrite(12, LOW);


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

    delay(500);

    battery_V = ESP.getVcc();
#ifndef DEBUG
  // if 5 minutes have elasped, go to sleep for 30 minutes
    if (lastMillis / 1000 > sleepTimeMin * 5 )
    {
      display.clearDisplay();
      display.display();
      delay(100);
      Serial.println("Going to sleep----------------------");

      ESP.deepSleep((sleepTimeMin * 30) * 1000000, WAKE_RF_DEFAULT);
      delay(100); // wait for deep sleep to happen ...
    }
#endif
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

  #ifdef SERIAL_DEBUG
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

  #ifdef SERIAL_DEBUG
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

  delay(1000);

  // Current
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Current: ");
  display.print(current);
  display.println(" mA");

  // Displays
  display.display();

}
// Default function for sending sensor data at intervals to Cayenne.
// You can also use functions for specific channels, e.g CAYENNE_OUT(1) for sending channel 1 data.
CAYENNE_OUT_DEFAULT()
{
  lastMillis = millis();
  // Write data to Cayenne here. This example just sends the current uptime in milliseconds on virtual channel 0.
  Cayenne.virtualWrite(0, lastMillis/1000);
//  Serial.println(lastMillis/1000);
  // Some examples of other functions you can use to send data.
  //Cayenne.celsiusWrite(1, 22.0);
  //Cayenne.luxWrite(2, 700);
  //Cayenne.virtualWrite(3, 50, TYPE_PROXIMITY, UNIT_CENTIMETER);
}

// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT()
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}
CAYENNE_IN(1)
{
   digitalWrite(LED_BUILTIN, !getValue.asInt());
}

CAYENNE_OUT(2)
{
   Cayenne.virtualWrite(2, current_mA);
}
//CAYENNE_OUT(3)
//{
//   Cayenne.virtualWrite(3,WiFi.localIP());
//   Serial.println(WiFi.localIP());
//}
CAYENNE_OUT (4)
{
   battery_V = (ESP.getVcc());
   Cayenne.virtualWrite(4, battery_V);
}
CAYENNE_OUT(5)
{
   Cayenne.virtualWrite(5, measurePower());

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
