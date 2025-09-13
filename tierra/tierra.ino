#include <SPI.h>
#include <LoRa.h>

// Pines del LoRa
#define SS      10
#define RST     9
#define DIO0    2

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Iniciando LoRa receptor...");

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) { // Frecuencia: 433 MHz
    Serial.println("Error iniciando LoRa!");
    while (true);
  }

  Serial.println("LoRa receptor listo!");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String msg = "";
    while (LoRa.available()) {
      msg += (char)LoRa.read();  // concatenas cada carácter recibido
    }
    Serial.println(msg);  // aquí se imprime con salto de línea
  }
}
