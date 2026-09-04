#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("I2C Scanner starting...");
  Wire.begin();

  byte error, address;
  int devices = 0;

  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      devices++;
    }
  }

  if (devices == 0)
    Serial.println("No I2C devices found");
  else
    Serial.println("Scan done");
}

void loop() {}