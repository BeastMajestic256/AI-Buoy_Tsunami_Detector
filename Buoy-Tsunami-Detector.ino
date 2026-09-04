/*
AI-POWERED ROBOTIC BUOY
Tsunami Detection Prototype
Arduino UNO R3

Sensors:
- MPU-6050 IMU
- MS5803 Pressure Sensor

Communications:
- LoRa SX127x

Controls:
- Push Button
*/

#include <Wire.h>
#include <SPI.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include <SparkFun_MS5803_I2C.h>

#include <LoRa.h>

// ============================================================
// PIN CONFIGURATION
// ============================================================

#define BUTTON_PIN 6

// LoRa
#define LORA_SS_PIN     10
#define LORA_RESET_PIN   9

// ============================================================
// LORA FREQUENCY
// ============================================================
#define LORA_FREQUENCY 915E6   // Change if needed (433E6 / 868E6 / 915E6)

// ============================================================
// SENSOR OBJECTS
// ============================================================

Adafruit_MPU6050 mpu;
MS5803 pressureSensor(ADDRESS_HIGH);   // 0x76

// ============================================================
// SYSTEM SETTINGS
// ============================================================

const unsigned long SENSOR_INTERVAL = 1000;   // 1 second

// Anomaly thresholds
const float PRESSURE_CHANGE_THRESHOLD = 3.0;  // mbar
const float ACCEL_CHANGE_THRESHOLD = 2.0;     // m/s²

// ============================================================
// VARIABLES
// ============================================================

unsigned long lastSensorRead = 0;
float previousPressure = 0.0;

bool pressureSensorOK = false;
bool mpuOK = false;
bool loraOK = false;

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println(F("=============================================="));
  Serial.println(F(" AI-POWERED ROBOTIC BUOY"));
  Serial.println(F(" Tsunami Detection Prototype"));
  Serial.println(F(" Arduino UNO R3"));
  Serial.println(F("=============================================="));

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin();

  // ---------- MPU-6050 ----------
  Serial.println(F("\nInitializing MPU-6050..."));
  if (!mpu.begin()) {
    Serial.println(F("ERROR: MPU-6050 not detected!"));
    mpuOK = false;
  } else {
    Serial.println(F("MPU-6050 OK"));
    mpuOK = true;
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  // ---------- MS5803 ----------
  Serial.println(F("\nInitializing MS5803..."));
  pressureSensor.reset();
  if (pressureSensor.begin()) {
    Serial.println(F("MS5803 OK"));
    pressureSensorOK = true;
  } else {
    Serial.println(F("ERROR: MS5803 not detected!"));
    pressureSensorOK = false;
  }

  // ---------- LoRa ----------
  Serial.println(F("\nInitializing LoRa..."));
  LoRa.setPins(LORA_SS_PIN, LORA_RESET_PIN);

  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println(F("ERROR: LoRa initialization failed!"));
    loraOK = false;
  } else {
    Serial.println(F("LoRa OK"));
    loraOK = true;
    LoRa.setTxPower(17);
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
  }

  // ---------- Summary ----------
  Serial.println(F("\n----------------------------------------------"));
  Serial.println(F("SYSTEM READY"));
  Serial.println(F("----------------------------------------------"));
  Serial.print(F("MPU6050: ")); Serial.println(mpuOK ? F("OK") : F("FAIL"));
  Serial.print(F("MS5803:  ")); Serial.println(pressureSensorOK ? F("OK") : F("FAIL"));
  Serial.print(F("LoRa:    ")); Serial.println(loraOK ? F("OK") : F("FAIL"));
  Serial.println(F("----------------------------------------------\n"));
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop() {
  unsigned long currentTime = millis();

  // Manual button
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println(F("\nMANUAL ALERT BUTTON PRESSED!"));
    sendLoRaAlert("MANUAL ALERT - BUOY TEST BUTTON");
    delay(1500);
  }

  // Sensor sampling
  if (currentTime - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = currentTime;
    readSensors();
  }
}

// ============================================================
// READ SENSORS
// ============================================================

void readSensors() {
  float ax = 0, ay = 0, az = 0;
  float gx = 0, gy = 0, gz = 0;
  float pressure = 0;

  // MPU-6050
  if (mpuOK) {
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);
    ax = accel.acceleration.x;
    ay = accel.acceleration.y;
    az = accel.acceleration.z;
    gx = gyro.gyro.x;
    gy = gyro.gyro.y;
    gz = gyro.gyro.z;
  }

  // MS5803
  if (pressureSensorOK) {
    pressure = pressureSensor.getPressure(ADC_4096);
  }

  // Display data
  Serial.println(F("========== SENSOR DATA =========="));
  Serial.print(F("ACC X: ")); Serial.print(ax, 3); Serial.println(F(" m/s2"));
  Serial.print(F("ACC Y: ")); Serial.print(ay, 3); Serial.println(F(" m/s2"));
  Serial.print(F("ACC Z: ")); Serial.print(az, 3); Serial.println(F(" m/s2"));
  Serial.print(F("GYRO X: ")); Serial.print(gx, 3); Serial.println(F(" rad/s"));
  Serial.print(F("GYRO Y: ")); Serial.print(gy, 3); Serial.println(F(" rad/s"));
  Serial.print(F("GYRO Z: ")); Serial.print(gz, 3); Serial.println(F(" rad/s"));
  Serial.print(F("PRESSURE: ")); Serial.print(pressure, 3); Serial.println(F(" mbar"));

  // CSV format (for later AI training)
  Serial.print(F("CSV,"));
  Serial.print(millis()); Serial.print(',');
  Serial.print(ax, 4); Serial.print(',');
  Serial.print(ay, 4); Serial.print(',');
  Serial.print(az, 4); Serial.print(',');
  Serial.print(gx, 4); Serial.print(',');
  Serial.print(gy, 4); Serial.print(',');
  Serial.print(gz, 4); Serial.print(',');
  Serial.println(pressure, 4);

  // Simple anomaly detection
  bool tsunamiCandidate = false;
  float pressureChange = abs(pressure - previousPressure);
  float accelerationMagnitude = sqrt(ax*ax + ay*ay + az*az);

  if (pressureSensorOK && previousPressure > 0 && pressureChange >= PRESSURE_CHANGE_THRESHOLD) {
    tsunamiCandidate = true;
    Serial.println(F("WARNING: RAPID PRESSURE CHANGE"));
  }

  if (mpuOK && accelerationMagnitude >= (9.81 + ACCEL_CHANGE_THRESHOLD)) {
    tsunamiCandidate = true;
    Serial.println(F("WARNING: ABNORMAL MOTION"));
  }

  if (tsunamiCandidate) {
    Serial.println(F("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
    Serial.println(F(" TSUNAMI CANDIDATE DETECTED"));
    Serial.println(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
    sendLoRaAlert("TSUNAMI_CANDIDATE");
  }

  previousPressure = pressure;

  // Send via LoRa
  sendLoRaSensorData(ax, ay, az, gx, gy, gz, pressure);
}

// ============================================================
// LORA FUNCTIONS
// ============================================================

void sendLoRaSensorData(float ax, float ay, float az,
                        float gx, float gy, float gz,
                        float pressure) {
  if (!loraOK) return;

  LoRa.beginPacket();
  LoRa.print(F("BUOY01,"));
  LoRa.print(millis()); LoRa.print(',');
  LoRa.print(ax, 3); LoRa.print(',');
  LoRa.print(ay, 3); LoRa.print(',');
  LoRa.print(az, 3); LoRa.print(',');
  LoRa.print(gx, 3); LoRa.print(',');
  LoRa.print(gy, 3); LoRa.print(',');
  LoRa.print(gz, 3); LoRa.print(',');
  LoRa.print(pressure, 2);
  LoRa.endPacket();

  Serial.println(F("LoRa sensor packet transmitted."));
}

void sendLoRaAlert(const char *message) {
  if (!loraOK) return;

  LoRa.beginPacket();
  LoRa.print(F("BUOY01,ALERT,"));
  LoRa.print(message);
  LoRa.endPacket();

  Serial.print(F("LoRa ALERT: "));
  Serial.println(message);
}