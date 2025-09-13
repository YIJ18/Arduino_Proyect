#include <LoRa.h>
#include <TinyGPS++.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <MPU9250.h>
#include <stdint.h>
#include <HardwareSerial.h>

// ------------------------
// Pines y configuración
// ------------------------

// Pines LoRa
#define SS 5
#define RST 14
#define DIO0 13

// LED de error
#define LED_PIN 2

// Pines I2C
#define I2C_SDA 21
#define I2C_SCL 22

// Pines OpenLog (Serial2)
#define OPENLOG_RX 35
#define OPENLOG_TX 33

// ------------------------
// Objetos Hardware
// ------------------------

// OpenLog
HardwareSerial openLogSerial(2);

// GPS
HardwareSerial gpsSerial(1);
TinyGPSPlus gps;

// Sensores
Adafruit_BME280 bme;
MPU9250 imu;

// ------------------------
// Estructura de datos
// ------------------------
typedef struct {
    float latitude;
    float longitude;
    float altitude;
    int32_t satellites; // tamaño garantizado
    float temp;
    float hum;
    float pres;
    float accX;
    float accY;
    float accZ;
    float gyroX;
    float gyroY;
    float gyroZ;
    float magX;
    float magY;
    float magZ;
} SensorData;

SensorData sensorData;

// ------------------------
// Variables para filtro MPU9250
// ------------------------
const int numReadings = 10;
float accXReadings[numReadings];
float accYReadings[numReadings];
float accZReadings[numReadings];
float gyroXReadings[numReadings];
float gyroYReadings[numReadings];
float gyroZReadings[numReadings];
int readingIndex = 0;

unsigned long lastPrintTime = 0;

// ------------------------
// Funciones auxiliares
// ------------------------

// Parpadeo de LED en caso de error
void errorBlink() {
    pinMode(LED_PIN, OUTPUT);
    while (true) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }
}

// Escribir datos en OpenLog
String writeToOpenLog(String data) {
    if (openLogSerial.availableForWrite()) {
        openLogSerial.println(data);
        return "SD: OK";
    } else {
        return "SD: ERR";
    }
}

// ------------------------
// Setup
// ------------------------
void setup() {
    Serial.begin(9600);

    // GPS
    gpsSerial.begin(9600);

    // OpenLog
    openLogSerial.begin(9600, SERIAL_8N1, OPENLOG_RX, OPENLOG_TX);

    // LoRa
    LoRa.setPins(SS, RST, DIO0);
    if (!LoRa.begin(433E6)) {
        Serial.println("Error iniciando LoRa");
        errorBlink();
    }
    Serial.println("LoRa + GPS iniciados");

    // I2C y sensores
    Wire.begin(I2C_SDA, I2C_SCL);

    if (!bme.begin(0x76)) {
        Serial.println("No se encontro el sensor BME280, verifique las conexiones o la direccion I2C.");
        errorBlink();
    }
    Serial.println("Sensor BME280 iniciado!");

    if (!imu.setup(0x68)) {
        Serial.println("No se pudo iniciar el MPU-9250/6500");
        errorBlink();
    }
    Serial.println("Sensor MPU-9250/6500 iniciado!");

    pinMode(LED_PIN, OUTPUT);

    Serial.println("Esperando datos...");

    openLogSerial.println("Iniciando registro de datos...");
}

// ------------------------
// Loop principal
// ------------------------
void loop() {
    // Procesar datos del GPS
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }

    // Enviar paquete cada segundo
    if (millis() - lastPrintTime > 1000) {
        lastPrintTime = millis();

        // --- BME280 ---
        sensorData.temp = bme.readTemperature();
        sensorData.hum  = bme.readHumidity();
        sensorData.pres = bme.readPressure() / 100.0F;

        // --- MPU9250 ---
        if (imu.update()) {
            // Actualizar búferes
            accXReadings[readingIndex] = imu.getAccX();
            accYReadings[readingIndex] = imu.getAccY();
            accZReadings[readingIndex] = imu.getAccZ();
            gyroXReadings[readingIndex] = imu.getGyroX();
            gyroYReadings[readingIndex] = imu.getGyroY();
            gyroZReadings[readingIndex] = imu.getGyroZ();
            readingIndex = (readingIndex + 1) % numReadings;

            // Promedio
            float avgAccX = 0, avgAccY = 0, avgAccZ = 0;
            float avgGyroX = 0, avgGyroY = 0, avgGyroZ = 0;
            for (int i = 0; i < numReadings; i++) {
                avgAccX += accXReadings[i];
                avgAccY += accYReadings[i];
                avgAccZ += accZReadings[i];
                avgGyroX += gyroXReadings[i];
                avgGyroY += gyroYReadings[i];
                avgGyroZ += gyroZReadings[i];
            }
            sensorData.accX = avgAccX / numReadings;
            sensorData.accY = avgAccY / numReadings;
            sensorData.accZ = avgAccZ / numReadings;
            sensorData.gyroX = avgGyroX / numReadings;
            sensorData.gyroY = avgGyroY / numReadings;
            sensorData.gyroZ = avgGyroZ / numReadings;
            sensorData.magX  = imu.getMagX();
            sensorData.magY  = imu.getMagY();
            sensorData.magZ  = imu.getMagZ();
        }

        // --- GPS ---
        if (gps.location.isValid()) {
            sensorData.latitude   = gps.location.lat();
            sensorData.longitude  = gps.location.lng();
            sensorData.altitude   = gps.altitude.meters();
            sensorData.satellites = gps.satellites.value();
        } else {
            sensorData.latitude   = 0.0;
            sensorData.longitude  = 0.0;
            sensorData.altitude   = 0.0;
            sensorData.satellites = 0;
        }

        // --- Enviar LoRa ---
        LoRa.beginPacket();
        LoRa.write((uint8_t*)&sensorData, sizeof(sensorData));
        LoRa.endPacket();

        // --- Guardar y mostrar ---
        String dataString = ""; // se puede usar para OpenLog
        String sdStatus = writeToOpenLog(dataString);
        String logLine = "Lat: " + String(sensorData.latitude, 6) +
                         ", Lon: " + String(sensorData.longitude, 6) +
                         ", Alt: " + String(sensorData.altitude, 2) +
                         ", Sats: " + String(sensorData.satellites) +
                         ", Temp: " + String(sensorData.temp, 2) +
                         ", Hum: " + String(sensorData.hum, 2) +
                         ", Pres: " + String(sensorData.pres, 2) +
                         ", AccX: " + String(sensorData.accX, 2) +
                         ", AccY: " + String(sensorData.accY, 2) +
                         ", AccZ: " + String(sensorData.accZ, 2) +
                         ", GyroX: " + String(sensorData.gyroX, 2) +
                         ", GyroY: " + String(sensorData.gyroY, 2) +
                         ", GyroZ: " + String(sensorData.gyroZ, 2) +
                         ", MagX: " + String(sensorData.magX, 2) +
                         ", MagY: " + String(sensorData.magY, 2) +
                         ", MagZ: " + String(sensorData.magZ, 2) +
                         " | " + sdStatus;
        Serial.println(logLine);
    }
}
