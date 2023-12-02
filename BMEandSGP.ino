#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Adafruit_SGP30.h"


Adafruit_BME280 bme;  // BME280 sensor
Adafruit_SGP30 sgp;   // SGP30 air quality sensor

#define SEALEVELPRESSURE_HPA (1013.25)

/* return absolute humidity [mg/m^3] with approximation formula
 * @param temperature [°C]
 * @param humidity [%RH]
 */
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
  // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
  const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
  return absoluteHumidityScaled;
}

void setup() {
  Serial.begin(9600);
  while (!Serial) { delay(10); } // Wait for serial console to open!

  Serial.println("BME280 and SGP30 test");

  // Initialize BME280
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    while (1);
  }

  // Initialize SGP30
  if (!sgp.begin(&Wire, 0x58)) {
    Serial.println("SGP30 sensor not found :(");
    while (1);
  }

  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
}

int counter = 0;

void loop() {
  // BME280 readings
  printBMEValues();

  // SGP30 readings
  if (!sgp.IAQmeasure()) {
    Serial.println("SGP30 Measurement failed");
    return;
  }
  Serial.print("SGP30 TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");

  // Other SGP30 raw measurements
  if (!sgp.IAQmeasureRaw()) {
    Serial.println("SGP30 Raw Measurement failed");
    return;
  }
  Serial.print("SGP30 Raw H2 "); Serial.print(sgp.rawH2); Serial.print(" \t");
  Serial.print("SGP30 Raw Ethanol "); Serial.print(sgp.rawEthanol); Serial.println("");

  delay(1000);

  counter++;
  if (counter == 30) {
    counter = 0;

    // SGP30 baseline values
    uint16_t TVOC_base, eCO2_base;
    if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get SGP30 baseline readings");
      return;
    }
    Serial.print("****SGP30 Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
  }
}

void printBMEValues() {
  Serial.print("BME280 Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" °C");

  Serial.print("BME280 Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("BME280 Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("BME280 Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
}
