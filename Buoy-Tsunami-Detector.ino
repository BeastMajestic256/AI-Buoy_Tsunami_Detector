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


#include <SoftwareSerial.h>

// Arduino RX, Arduino TX
SoftwareSerial sim800(10, 11);

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);

  Serial.println("SIM800L Test");
  Serial.println("----------------");
  Serial.println("Type AT commands into Serial Monitor.");
  Serial.println();

  delay(3000);

  sim800.println("AT");
}

void loop() {

  // Computer -> SIM800L
  if (Serial.available()) {
    sim800.write(Serial.read());
  }

  // SIM800L -> Computer
  if (sim800.available()) {
    Serial.write(sim800.read());
  }
}