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


#include <HX710.h>

#define HX710_DOUT 2
#define HX710_SCK  3

HX710 pressure;

void setup() {
  Serial.begin(19200);

  Serial.println("HX710B Pressure Sensor Test");

  // Initialize HX710B
  pressure.initialize(HX710_SCK, HX710_DOUT);

  Serial.println("HX710B initialized.");
}

void loop() {

  if (pressure.isReady()) {

    int32_t differential = pressure.getLastDifferentialInput();
    int32_t other = pressure.getLastOtherInput();

    Serial.print("Differential: ");
    Serial.print(differential);

    Serial.print(" | Other: ");
    Serial.println(other);

  } else {

    Serial.println("HX710B not ready.");

  }

  delay(500);
}