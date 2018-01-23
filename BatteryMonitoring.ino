#define WLAN_SSID       "...your SSID..."
#define WLAN_PASS       "...your password..."

#define AIO_KEY         "...your AIO key..."

// how often to report battery level to adafruit IO (in minutes)
#define BATTERY_INTERVAL 5

// how long to sleep between checking the door state (in seconds)
#define SLEEP_LENGTH 3

void setup() {
  Serial.begin(115200);
  Serial.println("HUZZAH Trigger Basic");
  
  EEPROM.begin(512);
  pinMode(DOOR, INPUT_PULLUP);

  // get the current count position from eeprom
  byte battery_count = EEPROM.read(0);

  // we only need this to happen once every X minutes,
  // so we use eeprom to track the count between resets.
  if(battery_count >= ((BATTERY_INTERVAL * 60) / SLEEP_LENGTH)) {
    // reset counter
    battery_count = 0;
    // report battery level to Adafruit IO
    battery_level();
  } else {
    // increment counter
    battery_count++;
  }

  // save the current count
  EEPROM.write(0, battery_count);
  EEPROM.commit();

  // if door isn't open, we don't need to send anything
  if(digitalRead(DOOR) == LOW) {
    Serial.println("Door closed");
    // we don't do anything
  } else {
    // the door is open if we have reached here,
    // so we should send a value to Adafruit IO.
    Serial.println("Door is open!");
    door_open();
  }

  // we are done here. go back to sleep.
  Serial.println("zzzz");
  ESP.deepSleep(SLEEP_LENGTH * 1000000, WAKE_RF_DISABLED);
}

void door_open() {

  // turn on wifi if we aren't connected
  if(WiFi.status() != WL_CONNECTED) {
    wifi_init();
  }
  
  // grab the door feed
  Adafruit_IO_Feed door = aio.getFeed("door");

  Serial.println("Sending to Adafruit.io");
  // send door open value to AIO
  door.send("1");

}

void battery_level() {

  // read the battery level from the ESP8266 analog in pin.
  // analog read level is 10 bit 0-1023 (0V-1V).
  // our 1M & 220K voltage divider takes the max
  // lipo value of 4.2V and drops it to 0.758V max.
  // this means our min analog read value should be 580 (3.14V)
  // and the max analog read value should be 774 (4.2V).
  int level = analogRead(A0);

  // convert battery level to percent
  level = map(level, 580, 774, 0, 100);
  Serial.print("Battery level: "); Serial.print(level); Serial.println("%");
  // turn on wifi if we aren't connected
  if(WiFi.status() != WL_CONNECTED)
    wifi_init();

  // grab the battery feed
  Adafruit_IO_Feed battery = aio.getFeed("battery");

  // send battery level to AIO
  battery.send(level);

}


