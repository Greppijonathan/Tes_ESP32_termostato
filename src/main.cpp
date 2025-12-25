#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "FS.h"
#include "SPIFFS.h"

// Librería para el manejo de energía del ESP32
#include "esp32-hal-cpu.h"

TFT_eSPI tft = TFT_eSPI();

// --- Configuración de Hardware ---
#define PIN_BL 32
#define PIN_RELE1 33
#define PIN_RELE2 26

// --- Paleta de Colores ---
#define COL_FONDO 0x0842
#define COL_CARD 0x10A4
#define COL_ACCENT 0x03EF
#define COL_BTN_ON 0x2661
#define COL_BTN_OFF 0x114F
#define COL_TEXTO 0xFFFF
#define COL_SUBTEXTO 0xAD75

// --- Variables de Estado ---
bool sistemaEstado = false;
bool pantallaEncendida = true;
unsigned long lastRelayMillis = 0;
bool estadoRele = false;

// --- Geometría de Botones ---
const int btnY = 250;
const int btnH = 60;
const int btnW = 105;
const int btn1X = 10;
const int btn2X = 125;
int cardH = 85;

// --- Prototipos ---
void dibujarInterfazBase();
void dibujarBotonSistema(bool estado);
void dibujarBotonBL();
void touch_calibrate();
void gestionarModoEnergia(bool despertar);
void actualizarVisualReles(); // Corregido: sin acento

void setup()
{
  Serial.begin(115200);

  // Configurar Pines de Relés
  pinMode(PIN_RELE1, OUTPUT);
  pinMode(PIN_RELE2, OUTPUT);
  digitalWrite(PIN_RELE1, LOW);
  digitalWrite(PIN_RELE2, HIGH); // Inician conmutados

  // Configurar Pin de Retroiluminación (Lógica PNP: LOW es ON)
  pinMode(PIN_BL, OUTPUT);
  digitalWrite(PIN_BL, LOW);

  if (!SPIFFS.begin(true))
    Serial.println("Error SPIFFS");

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  touch_calibrate();

  dibujarInterfazBase();
  dibujarBotonSistema(sistemaEstado);
  dibujarBotonBL();
  actualizarVisualReles();

  Serial.println("--- Dashboard Listo (Relés Activos) ---");
}

void loop()
{
  uint16_t x, y;

  // --- Lógica de Conmutación de Relés (Cada 3 segundos) ---
  if (millis() - lastRelayMillis >= 3000)
  {
    lastRelayMillis = millis();
    estadoRele = !estadoRele;

    digitalWrite(PIN_RELE1, estadoRele);
    digitalWrite(PIN_RELE2, !estadoRele);

    // Solo actualizamos la pantalla si está encendida
    if (pantallaEncendida)
    {
      actualizarVisualReles();
    }
  }

  if (tft.getTouch(&x, &y, 250))
  {
    // --- MODO SUSPENSIÓN ---
    if (!pantallaEncendida)
    {
      if ((x > btn2X) && (x < (btn2X + btnW)) && (y > btnY) && (y < (btnY + btnH)))
      {
        gestionarModoEnergia(true);
        actualizarVisualReles();
        Serial.println("WAKEUP: Restaurando frecuencias y pantalla");
        delay(300);
      }
      return;
    }

    // --- MODO ACTIVO ---
    else
    {
      if ((x > btn1X) && (x < (btn1X + btnW)) && (y > btnY) && (y < (btnY + btnH)))
      {
        sistemaEstado = !sistemaEstado;
        dibujarBotonSistema(sistemaEstado);
        Serial.print("TACTIL: ");
        Serial.println(sistemaEstado ? "ON" : "OFF");
        delay(350);
      }
      else if ((x > btn2X) && (x < (btn2X + btnW)) && (y > btnY) && (y < (btnY + btnH)))
      {
        Serial.println("SLEEP: Entrando en modo ahorro...");
        delay(200);
        gestionarModoEnergia(false);
      }
    }

    while (tft.getTouch(&x, &y, 250))
      ;
  }
}

void actualizarVisualReles()
{
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);

  // Rele 1
  tft.setTextColor(estadoRele ? COL_BTN_ON : COL_SUBTEXTO, COL_CARD);
  tft.drawString(estadoRele ? " ESTADO: ON " : " ESTADO: OFF", 62, 195, 2);

  // Rele 2
  tft.setTextColor(!estadoRele ? COL_BTN_ON : COL_SUBTEXTO, COL_CARD);
  tft.drawString(!estadoRele ? " ESTADO: ON " : " ESTADO: OFF", 177, 195, 2);
}

void gestionarModoEnergia(bool despertar)
{
  if (despertar)
  {
    setCpuFrequencyMhz(240);
    tft.writecommand(0x11);
    delay(120);
    digitalWrite(PIN_BL, HIGH); // PNP ON
    pantallaEncendida = true;
  }
  else
  {
    digitalWrite(PIN_BL, LOW); // PNP OFF
    tft.writecommand(0x10);
    setCpuFrequencyMhz(80);
    pantallaEncendida = false;
  }
}

void dibujarBotonSistema(bool estado)
{
  uint16_t color = estado ? COL_BTN_ON : COL_BTN_OFF;
  tft.fillRoundRect(btn1X, btnY, btnW, btnH, 8, color);
  tft.drawRoundRect(btn1X, btnY, btnW, btnH, 8, COL_ACCENT);
  tft.setTextColor(COL_TEXTO);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(estado ? "TACTIL ON" : "TACTIL OFF", btn1X + (btnW / 2), btnY + (btnH / 2), 2);
}

void dibujarBotonBL()
{
  tft.fillRoundRect(btn2X, btnY, btnW, btnH, 8, COL_CARD);
  tft.drawRoundRect(btn2X, btnY, btnW, btnH, 8, COL_ACCENT);
  tft.setTextColor(COL_TEXTO);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("SLEEP", btn2X + (btnW / 2), btnY + (btnH / 2), 2);
}

void dibujarInterfazBase()
{
  tft.fillScreen(COL_FONDO);
  tft.fillRect(0, 0, 240, 40, COL_CARD);
  tft.drawFastHLine(0, 40, 240, COL_ACCENT);
  tft.setTextColor(COL_TEXTO);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("PANEL DE CONTROL", 120, 20, 2);
  tft.drawRoundRect(10, 55, 105, cardH, 8, TFT_WHITE);
  tft.drawRoundRect(125, 55, 105, cardH, 8, TFT_WHITE);
  tft.drawRoundRect(10, 150, 105, cardH, 8, TFT_WHITE);
  tft.drawRoundRect(125, 150, 105, cardH, 8, TFT_WHITE);
  tft.setTextColor(COL_SUBTEXTO);
  tft.drawString("Sensor 1", 62, 70);
  tft.drawString("Sensor 2", 177, 70);
  tft.drawString("Rele 1", 62, 165);
  tft.drawString("Rele 2", 177, 165);
}

void touch_calibrate()
{
  uint16_t calData[5];
  if (SPIFFS.exists("/TouchCalData1"))
  {
    fs::File f = SPIFFS.open("/TouchCalData1", "r");
    if (f)
    {
      f.readBytes((char *)calData, 14);
      f.close();
      tft.setTouch(calData);
    }
  }
}