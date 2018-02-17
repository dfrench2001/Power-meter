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
#define AIO_FEED_power                "power"
#define AIO_FEED_current              "current"
#define AIO_FEED_battery_level        "Battery level"
#define AIO_FEED_counter              "counter"


//AdafruitIO_WiFi io(AIO_USERNAME, AIO_KEY, SSID, NETWORK_PASS);

//WiFi connection;
/* set up the feeds to AdafruitIO
 * AdafruitIO_Feed *power_feed = io.feed("power");
 * AdafruitIO_Feed *current_feed = io.feed("current");
 * AdafruitIO_Feed *Battery_level_feed = io.feed("Battery level");
 * AdafruitIO_Feed *counter_feed = io.feed("counter");
 * AdafruitIO_Feed *LED_feed = io.feed("LED");
 */
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
unsigned long sleepTimeS = 1000;
unsigned long sleepTimeMin = 15 * 1000;  // 1000 milliseconds * 60 secondes = 1 minute
unsigned long sleepTimeHour = sleepTimeMin * 60 ;

Metro feedMetro = Metro(sleepTimeMin);  // 10 min timer to update AdafruitIO feeds
Metro sleepMetro = Metro(sleepTimeMin * 5); // 5 min timer to go in sleep mode

unsigned long lastMillis = 0;
uint8_t firsttime = 1;

//------------------------------------------------ Setup ------------------------------------------------
void setup()   {

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  //  Setup OVER THE AIR (wifi) sketch updates
  OTA_Setup();
  myESP.setMQTTCallback(callback);
  myESP.addSubscription(AIO_USERNAME"/feeds/"AIO_FEED_power);
  myESP.addSubscription(AIO_USERNAME"/feeds/"AIO_FEED_current);
  myESP.addSubscription(AIO_USERNAME"/feeds/"AIO_FEED_battery_level);
  myESP.addSubscription(AIO_USERNAME"/feeds/"AIO_FEED_counter);


  // connect to io.adafruit.com
//  io.connect();

  //setup ESPHelper
//  myESP.begin();

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
//------------------------------------------loop------------------------------------------------------

void loop() {
Serial.println("looping-----------------");
Serial.println(firsttime);

  if(firsttime == 0)
  {
    Serial.println("calling myESP.loop ");
//    myESP.loop();  //run the loop() method as often as possible - this keeps the network services running
    Serial.print("current loop status =" ); Serial.println(myESP.loop());
  }

   lastMillis = millis() /1000;

  // External LED off
  digitalWrite(12, HIGH);

  // setup handler for LED actions
//  LED_feed->onMessage(handleLED);

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

    delay(2000);

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

   battery_V = (ESP.getVcc() /1000);

  // save to the feed on Adafruit IO
  if(feedMetro.check() == 1)
  {
    if (firsttime == 1 )
    { Serial.println("calling myESP begin");
			myESP.begin();
    }
		else
		{ Serial.println("calling Wifi.begin"); 
			WiFi.begin(homeNet.ssid, homeNet.pass); // need to check return values
		}
     // wait for a connection
    Serial.println("waiting for my connection");
    while(WiFi.status() != WL_CONNECTED ){
    Serial.print(".");Serial.println(WiFi.status());
    WiFi.printDiag(Serial);
    delay(2000);
  }

     // we are connected
    Serial.print("current status =" ); Serial.println(myESP.loop());

   //generate a string from our counter variable

    char pubString[10];
    itoa(lastMillis, pubString, 10);
    myESP.publish(AIO_USERNAME"/feeds/"AIO_FEED_counter, pubString);
    memset(pubString, 0x20, sizeof(pubString));
    delay(250);
Serial.print(AIO_USERNAME"/feeds/"AIO_FEED_counter" ");Serial.println(pubString);

    //publish the data to MQTT
    ftoa( pubString,current_mA, 10);
    myESP.publish(AIO_USERNAME"/feeds/"AIO_FEED_current, pubString);
    memset(pubString, 0x20, sizeof(pubString));
    delay(250);
Serial.print(AIO_USERNAME"/feeds/"AIO_FEED_current" "); Serial.println( pubString);

    ftoa(pubString,power_mW, 10);
    myESP.publish(AIO_USERNAME"/feeds/"AIO_FEED_power, pubString);
    memset(pubString, 0x20, sizeof(pubString));
    delay(250);
Serial.print(AIO_USERNAME"/feeds/"AIO_FEED_power" "); Serial.println( pubString);

    ftoa(pubString, battery_V, 10);
    myESP.publish(AIO_USERNAME"/feeds/"AIO_FEED_battery_level, pubString);
    memset(pubString, 0x20, sizeof(pubString));
    delay(250);
Serial.print(AIO_USERNAME"/feeds/"AIO_FEED_battery_level" "); Serial.println( pubString);

//    current_feed->save(current_mA);
//    power_feed->save(power_mW);
//    Battery_level_feed->save(battery_V / 1000);
//    counter_feed->save(lastMillis );


    Serial.print("current status before END =" ); Serial.println(myESP.loop());
    if (firsttime == 1)
    {
      firsttime = 0;
      myESP.end();
    }
    else
    {
      WiFi.disconnect();
    }

    Serial.print("current status after END =" ); Serial.println(myESP.loop());
  }

   yield();
}

//------------------------------------------ FUNCTIONS ---------------------------------------------------


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

//	myESP.begin();
  delay(500);
}
void callback(char* topic, uint8_t* payload, unsigned int length) {
	//put mqtt callback code here

  // Serial.print(topic, payload,length);
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

char *ftoa(char *buffer, double d, int precision) {

  long wholePart = (long) d;

  // Deposit the whole part of the number.

  itoa(wholePart,buffer,10);

  // Now work on the faction if we need one.

  if (precision > 0) {

    // We do, so locate the end of the string and insert
    // a decimal point.

    char *endOfString = buffer;
    while (*endOfString != '\0') endOfString++;
    *endOfString++ = '.';

    // Now work on the fraction, be sure to turn any negative
    // values positive.

    if (d < 0) {
      d *= -1;
      wholePart *= -1;
    }

    double fraction = d - wholePart;
    while (precision > 0) {

      // Multipleby ten and pull out the digit.

      fraction *= 10;
      wholePart = (long) fraction;
      *endOfString++ = '0' + wholePart;

      // Update the fraction and move on to the
      // next digit.

      fraction -= wholePart;
      precision--;
    }

    // Terminate the string.

    *endOfString = '\0';
  }

   return buffer;
}
