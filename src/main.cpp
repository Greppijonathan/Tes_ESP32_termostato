#include <Arduino.h>

void setup()
{
  // Inicializa la comunicación serial a 115200 baudios
  Serial.begin(115200);
  delay(1000);
  Serial.println(">>> ESP32 Encendido y Correcto <<<");
}

void loop()
{
  Serial.println("Probando UART... El micro está vivo.");
  delay(2000);
}