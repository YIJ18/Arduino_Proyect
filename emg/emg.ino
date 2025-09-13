const int pinEMG = 4;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int signalEMG = analogRead(pinEMG);
  Serial.println(signalEMG);
  delay(15);
}
