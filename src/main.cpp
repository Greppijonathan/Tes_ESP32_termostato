#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "FS.h"
#include "SPIFFS.h"

TFT_eSPI tft = TFT_eSPI();

// --- Paleta de Colores "Dark Mode" ---
#define COL_FONDO 0x0842    // Gris muy oscuro
#define COL_CARD 0x10A4     // Gris azulado
#define COL_ACCENT 0x03EF   // Cyan
#define COL_BTN_ON 0x2661   // Verde
#define COL_BTN_OFF 0x114F  // Azul oscuro
#define COL_TEXTO 0xFFFF    // Blanco
#define COL_SUBTEXTO 0xAD75 // Gris claro

// --- Variables de Estado ---
bool botonEstado = false;

// --- Geometría ---
const int btnX = 20, btnY = 250, btnW = 200, btnH = 60;
int cardH = 90;

// --- Prototipos ---
void dibujarInterfazBase();
void dibujarBoton(bool estado);
void touch_calibrate();

void setup()
{
  // 1. PIN RESET HARDWARE (GPIO 4) - Antes que cualquier otra cosa
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH); // Estado normal
  delay(100);
  digitalWrite(4, LOW);  // Reset activo
  delay(500);            // Tiempo suficiente para descargar capacitores
  digitalWrite(4, HIGH); // Liberar reset
  delay(500);            // Esperar a que el controlador ILI9341 despierte

  Serial.begin(115200);

  // 2. Inicialización de periféricos
  if (!SPIFFS.begin(true))
  {
    Serial.println("Error SPIFFS");
  }

  // 3. Inicializar TFT
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK); // Limpieza inicial

  touch_calibrate();

  dibujarInterfazBase();
  dibujarBoton(botonEstado);

  Serial.println("--- Sistema Iniciado Correctamente ---");
}

void loop()
{
  uint16_t x, y;
  if (tft.getTouch(&x, &y, 250))
  {
    if ((x > btnX) && (x < (btnX + btnW)) && (y > btnY) && (y < (btnY + btnH)))
    {
      botonEstado = !botonEstado;
      dibujarBoton(botonEstado);

      Serial.print("Boton: ");
      Serial.println(botonEstado ? "ON" : "OFF");

      delay(350);
      while (tft.getTouch(&x, &y, 250))
        ;
    }
  }
}

// Función auxiliar para dibujar marcos con borde de 2px
void dibujarMarcoConBorde(int x, int y, int w, int h)
{
  // 1. Relleno de la tarjeta
  tft.fillRoundRect(x, y, w, h, 8, COL_CARD);
  // 2. Borde exterior (Blanco)
  tft.drawRoundRect(x, y, w, h, 8, TFT_WHITE);
  // 3. Segundo borde interior para dar grosor (Blanco)
  tft.drawRoundRect(x + 1, y + 1, w - 2, h - 2, 8, TFT_WHITE);
}

void dibujarInterfazBase()
{
  tft.fillScreen(COL_FONDO);

  // 1. Cabecera (Actualizada)
  tft.fillRect(0, 0, 240, 40, COL_CARD);
  tft.drawFastHLine(0, 40, 240, COL_ACCENT);
  tft.setTextColor(COL_TEXTO);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CONTROL DE TEMPERATURA", 120, 20, 2);

  // 2. Dibujar los 4 Campos con borde blanco de 2px
  dibujarMarcoConBorde(10, 50, 105, cardH);   // Slot 1
  dibujarMarcoConBorde(125, 50, 105, cardH);  // Slot 2
  dibujarMarcoConBorde(10, 150, 105, cardH);  // Slot 3
  dibujarMarcoConBorde(125, 150, 105, cardH); // Slot 4

  // Etiquetas
  tft.setTextColor(COL_SUBTEXTO);
  tft.setTextFont(2);
  tft.drawString("SENSOR 1", 62, 65);
  tft.drawString("SENSOR 2", 177, 65);
  tft.drawString("RELE 1", 62, 165);
  tft.drawString("RELE 2", 177, 165);
}

void dibujarBoton(bool estado)
{
  uint16_t color = estado ? COL_BTN_ON : COL_BTN_OFF;
  String txt = estado ? "BOTON ON" : "BOTON OFF";

  tft.fillRoundRect(btnX, btnY, btnW, btnH, 12, color);
  // Borde del botón en Cyan para que resalte del resto
  tft.drawRoundRect(btnX, btnY, btnW, btnH, 12, COL_ACCENT);
  tft.drawRoundRect(btnX + 1, btnY + 1, btnW - 2, btnH - 2, 12, COL_ACCENT);

  tft.setTextColor(COL_TEXTO, color);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(txt, btnX + (btnW / 2), btnY + (btnH / 2), 4);
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