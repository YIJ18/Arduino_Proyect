#include <SPI.h>
#include <LoRa.h>

#define SS 5      // CS
#define RST 14    // RESET
#define DIO0 26   // DIO0

void setup() {
  Serial.begin(115200);
  while (!Serial);

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) {  // Ajusta la frecuencia a tu módulo
    Serial.println("Error iniciando LoRa");
    while (true);
  }

  Serial.println("LoRa iniciado como receptor");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
    Serial.print("Mensaje recibido: ");
    Serial.println(incoming);
  }
}