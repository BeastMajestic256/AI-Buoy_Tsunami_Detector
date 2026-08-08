/*
Buoy-Tsunami-Detector.ino

Programmed by John Nhelmer S. Nuguid

with

Sean Andrei DJ. Delima
Chris Laurence Lacsamana

Organizations from
9 - Einstein
CAHS
*/


#include <Wire.h>

const int MPU = 0x68;

void setup() {
  Serial.begin(19200);
  Wire.begin();

  // Wake up MPU6050
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  byte error = Wire.endTransmission(true);

  if (error == 0) {
    Serial.println("MPU6050 connected successfully.");
  } else {
    Serial.print("MPU6050 communication error: ");
    Serial.println(error);
  }

  delay(1000);
}

void loop() {

  // Request accelerometer data
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU, 6, true);

  if (Wire.available() == 6) {

    int16_t rawX = Wire.read() << 8 | Wire.read();
    int16_t rawY = Wire.read() << 8 | Wire.read();
    int16_t rawZ = Wire.read() << 8 | Wire.read();

    float X = rawX / 16384.0;
    float Y = rawY / 16384.0;
    float Z = rawZ / 16384.0;

    Serial.print("X: ");
    Serial.print(X, 3);

    Serial.print(" | Y: ");
    Serial.print(Y, 3);

    Serial.print(" | Z: ");
    Serial.println(Z, 3);

  } else {
    Serial.println("ERROR: Sensor data unavailable.");
  }

  delay(250);
}