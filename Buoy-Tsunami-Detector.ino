#define HX_DOUT 3
#define HX_SCK  4

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(HX_DOUT, INPUT);
  pinMode(HX_SCK, OUTPUT);
  digitalWrite(HX_SCK, LOW);
  
  Serial.println("HX710B Test Starting...");
  Serial.println("If you see changing numbers → sensor is working");
  Serial.println("If you only see 0 → check wiring or power");
}

long readHX710B() {
  // Wait for ready (DOUT goes LOW)
  unsigned long timeout = millis();
  while (digitalRead(HX_DOUT) == HIGH) {
    if (millis() - timeout > 300) {
      return 0;   // timeout
    }
  }

  unsigned long value = 0;

  // Read 24 bits
  for (int i = 0; i < 24; i++) {
    digitalWrite(HX_SCK, HIGH);
    delayMicroseconds(1);
    value = value << 1;
    digitalWrite(HX_SCK, LOW);
    delayMicroseconds(1);
    if (digitalRead(HX_DOUT)) {
      value++;
    }
  }

  // One extra pulse (channel A, gain 128)
  digitalWrite(HX_SCK, HIGH);
  delayMicroseconds(1);
  digitalWrite(HX_SCK, LOW);

  // Convert 24-bit two's complement
  if (value & 0x800000UL) {
    value |= 0xFF000000UL;
  }

  return (long)value;
}

void loop() {
  long raw = readHX710B();
  
  Serial.print("Raw pressure value: ");
  Serial.println(raw);
  
  delay(500);
}