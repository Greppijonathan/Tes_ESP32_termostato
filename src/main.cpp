#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "FS.h"
#include "SPIFFS.h"

TFT_eSPI tft = TFT_eSPI();

// --- Configuración de Hardware ---
#define PIN_BL 32 // Base del BC558 (PNP)

// --- Paleta de Colores ---
#define COL_FONDO 0x0842
#define COL_CARD 0x10A4
#define COL_ACCENT 0x03EF
#define COL_BTN_ON 0x2661  // Verde
#define COL_BTN_OFF 0x114F // Azul oscuro
#define COL_TEXTO 0xFFFF
#define COL_SUBTEXTO 0xAD75

// --- Variables de Estado ---
bool sistemaEstado = false;
bool pantallaEncendida = true;

// --- Geometría de Botones ---
const int btnY = 250;
const int btnH = 60;
const int btnW = 105;
const int btn1X = 10;  // SISTEMA (Izquierda)
const int btn2X = 125; // BL SLEEP (Derecha)
int cardH = 85;

// --- Prototipos ---
void dibujarInterfazBase();
void dibujarBotonSistema(bool estado);
void dibujarBotonBL();
void touch_calibrate();
void toggleBacklight(bool encender);

void setup()
{
  pinMode(PIN_BL, OUTPUT);
  toggleBacklight(true); // Lógica PNP: LOW es encendido

  Serial.begin(115200);
  if (!SPIFFS.begin(true))
    Serial.println("Error SPIFFS");

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  touch_calibrate();

  dibujarInterfazBase();
  dibujarBotonSistema(sistemaEstado);
  dibujarBotonBL();

  Serial.println("--- Sistema Inicializado ---");
}

void loop()
{
  uint16_t x, y;

  // 1. Verificamos si hay un toque en la pantalla
  if (tft.getTouch(&x, &y, 250))
  {

    // --- ESTADO: PANTALLA APAGADA ---
    if (!pantallaEncendida)
    {
      // Solo despertamos si el toque es en el área del botón BL (Derecho)
      if ((x > btn2X) && (x < (btn2X + btnW)) && (y > btnY) && (y < (btnY + btnH)))
      {
        toggleBacklight(true);
        Serial.println("WAKE UP: Pantalla Encendida");
        delay(300); // Pequeña espera para evitar rebotes
      }
      // Mientras esté apagada, no hacemos nada más
    }

    // --- ESTADO: PANTALLA ENCENDIDA ---
    else
    {
      // BOTÓN 1: SISTEMA (Izquierda)
      if ((x > btn1X) && (x < (btn1X + btnW)) && (y > btnY) && (y < (btnY + btnH)))
      {
        sistemaEstado = !sistemaEstado;
        dibujarBotonSistema(sistemaEstado);
        Serial.print("SISTEMA: ");
        Serial.println(sistemaEstado ? "ON" : "OFF");
        delay(350);
      }

      // BOTÓN 2: BL SLEEP (Derecha)
      else if ((x > btn2X) && (x < (btn2X + btnW)) && (y > btnY) && (y < (btnY + btnH)))
      {
        Serial.println("SLEEP: Apagando pantalla");
        toggleBacklight(false);
        delay(350);
      }
    }

    // Bloqueo para que no se repita la acción mientras se mantiene presionado
    while (tft.getTouch(&x, &y, 250))
      ;
  }
}

// --- Lógica del Transistor PNP (BC558) ---
void toggleBacklight(bool encender)
{
  if (encender)
  {
    digitalWrite(PIN_BL, HIGH); 
    pantallaEncendida = true;
  }
  else
  {
    digitalWrite(PIN_BL, LOW);
    pantallaEncendida = false;
  }
}

// --- Funciones de Interfaz ---

void dibujarBotonSistema(bool estado)
{
  uint16_t color = estado ? COL_BTN_ON : COL_BTN_OFF;
  String txt = estado ? "SISTEMA ON" : "SISTEMA OFF";

  tft.fillRoundRect(btn1X, btnY, btnW, btnH, 8, color);
  tft.drawRoundRect(btn1X, btnY, btnW, btnH, 8, COL_ACCENT);

  tft.setTextColor(COL_TEXTO);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(txt, btn1X + (btnW / 2), btnY + (btnH / 2), 2);
}

void dibujarBotonBL()
{
  tft.fillRoundRect(btn2X, btnY, btnW, btnH, 8, COL_CARD);
  tft.drawRoundRect(btn2X, btnY, btnW, btnH, 8, COL_ACCENT);

  tft.setTextColor(COL_TEXTO);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("BL SLEEP", btn2X + (btnW / 2), btnY + (btnH / 2), 2);
}

void dibujarInterfazBase()
{
  tft.fillScreen(COL_FONDO);
  tft.fillRect(0, 0, 240, 40, COL_CARD);
  tft.drawFastHLine(0, 40, 240, COL_ACCENT);
  tft.setTextColor(COL_TEXTO);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("PANEL DE CONTROL", 120, 20, 2);

  // Cuadros de sensores
  tft.drawRoundRect(10, 55, 105, cardH, 8, TFT_WHITE);
  tft.drawRoundRect(125, 55, 105, cardH, 8, TFT_WHITE);
  tft.drawRoundRect(10, 150, 105, cardH, 8, TFT_WHITE);
  tft.drawRoundRect(125, 150, 105, cardH, 8, TFT_WHITE);

  tft.setTextColor(COL_SUBTEXTO);
  tft.drawString("S1", 62, 70);
  tft.drawString("S2", 177, 70);
  tft.drawString("R1", 62, 165);
  tft.drawString("R2", 177, 165);
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