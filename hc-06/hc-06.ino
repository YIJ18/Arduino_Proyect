#include <SoftwareSerial.h>

SoftwareSerial BT(10, 11); // RX, TX

void setup() {
  Serial.begin(9600);   // Para comunicarte con el monitor serial
  BT.begin(9600);       // Baud rate por defecto del HC-06 (puede ser 38400)
  Serial.println("Enviando comandos AT al HC-06...");
}

void loop() {
  if (Serial.available()) {
    BT.write(Serial.read()); // Lo que escribas se va al HC-06
  }
  if (BT.available()) {
    Serial.write(BT.read()); // Lo que responda el HC-06 lo ves en pantalla
  }
}
