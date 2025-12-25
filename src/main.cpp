#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "FS.h"
#include "SPIFFS.h"

TFT_eSPI tft = TFT_eSPI();

// --- Configuración de Pines ---
#define PIN_BL 32  // Pin de Retroiluminación

// --- Paleta de Colores ---
#define COL_FONDO 0x0842  
#define COL_CARD 0x10A4   
#define COL_ACCENT 0x03EF  
#define COL_BTN_ON 0x2661  
#define COL_BTN_OFF 0x114F 
#define COL_TEXTO 0xFFFF   
#define COL_SUBTEXTO 0xAD75 

// --- Variables de Estado ---
bool botonEstado = false;
const int btnX = 20, btnY = 250, btnW = 200, btnH = 60;
int cardH = 90;

// --- Prototipos ---
void dibujarInterfazBase();
void dibujarBoton(bool estado);
void touch_calibrate();
void dibujarMarcoConBorde(int x, int y, int w, int h);

void setup()
{
  // 1. CONTROL DE RETROILUMINACIÓN (Backlight)
  // Lo encendemos inmediatamente para que la pantalla sea visible
  pinMode(PIN_BL, OUTPUT);
  digitalWrite(PIN_BL, HIGH); 

  // 2. ESPERA DE ESTABILIZACIÓN
  // Damos tiempo al circuito RC del pin EN y a la fuente de poder
  delay(1000); 

  Serial.begin(115200);

  // 3. INICIALIZACIÓN DE ARCHIVOS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error SPIFFS");
  }

  // 4. INICIALIZAR PANTALLA
  // Como RST está en EN, tft.init() se encarga solo de la comunicación SPI
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK); 

  touch_calibrate();
  dibujarInterfazBase();
  dibujarBoton(botonEstado);

  Serial.println("--- Dashboard Listo (Backlight ON) ---");
}

void loop()
{
  uint16_t x, y;
  // Ajustamos el umbral de presión a 250
  if (tft.getTouch(&x, &y, 250))
  {
    if ((x > btnX) && (x < (btnX + btnW)) && (y > btnY) && (y < (btnY + btnH)))
    {
      botonEstado = !botonEstado;
      dibujarBoton(botonEstado);

      Serial.print("Boton: ");
      Serial.println(botonEstado ? "ON" : "OFF");

      delay(350);
      while (tft.getTouch(&x, &y, 250));
    }
  }
}

// --- FUNCIONES DE DIBUJO ---

void dibujarMarcoConBorde(int x, int y, int w, int h)
{
  tft.fillRoundRect(x, y, w, h, 8, COL_CARD);
  tft.drawRoundRect(x, y, w, h, 8, TFT_WHITE);
  tft.drawRoundRect(x + 1, y + 1, w - 2, h - 2, 8, TFT_WHITE);
}

void dibujarInterfazBase()
{
  tft.fillScreen(COL_FONDO);

  // Cabecera
  tft.fillRect(0, 0, 240, 40, COL_CARD);
  tft.drawFastHLine(0, 40, 240, COL_ACCENT);
  tft.setTextColor(COL_TEXTO);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Control de temperatura", 120, 20, 2);

  // Slots con bordes de 2px
  dibujarMarcoConBorde(10, 50, 105, cardH);   
  dibujarMarcoConBorde(125, 50, 105, cardH);  
  dibujarMarcoConBorde(10, 150, 105, cardH);  
  dibujarMarcoConBorde(125, 150, 105, cardH); 

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
  String txt = estado ? "SISTEMA ON" : "SISTEMA OFF";

  tft.fillRoundRect(btnX, btnY, btnW, btnH, 12, color);
  tft.drawRoundRect(btnX, btnY, btnW, btnH, 12, COL_ACCENT);
  tft.drawRoundRect(btnX + 1, btnY + 1, btnW - 2, btnH - 2, 12, COL_ACCENT);

  tft.setTextColor(COL_TEXTO);
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