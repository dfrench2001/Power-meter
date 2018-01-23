#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

void setup()
{
 uint32_t currentFrequency;

 Serial.begin(115200);

 Serial.println("Communication test with with INA219 ...");
 ina219.begin();
 ina219.setCalibration_16V_400mA();

//   ina219.setCalibration_32V_2A();
}

void loop()
{
 float shuntvoltage = 0;

shuntvoltage = ina219.getShuntVoltage_mV();

 Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
 Serial.println("");

delay(100);
}
