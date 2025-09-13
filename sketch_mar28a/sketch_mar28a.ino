int SensorPin = A0;

void setup() {
  pinMode(7, OUTPUT);
  Serial.begin(9600);
}

void loop() {  
  int humedad = analogRead(SensorPin);
  int humedad_porcentaje = 100 - ((humedad * 100) / 1023); // Conversión a porcentaje

  Serial.print("Humedad: ");
  Serial.print(humedad_porcentaje);
  Serial.println("%");

  if (humedad >= 460) {
    digitalWrite(7, LOW);
  } else {
    digitalWrite(7, HIGH);
  }

  delay(1000);
}
