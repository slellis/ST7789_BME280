#include <Adafruit_GFX.h>         // Biblioteca gráfica base
#include <Adafruit_ST7789.h>      // Controle do display ST7789
#include <Adafruit_Sensor.h>      // Base para sensores Adafruit
#include <Adafruit_BME280.h>      // Biblioteca para o BME280
#include <Wire.h>                 // Comunicação I2C
#include <SPI.h>                  // Comunicação SPI
#include <Fonts/FreeSans9pt7b.h>  // Fonte elegante para os textos

// Definições dos pinos do display ST7789
#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_CS     5
#define TFT_DC    19
#define TFT_RST    4

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Pinos I2C do sensor BME280
#define BME_SDA  21
#define BME_SCL  22
Adafruit_BME280 bme;

// Altitude local para cálculo da pressão ao nível do mar
#define ALTITUDE_LOCAL 1018.0

// Variáveis para histórico da pressão e controle de atualização
float history[40];
int historyIndex = 0;
float lastTemp = -999, lastHum = -999, lastPres = -999, lastPresNmm = -999;

// Desenha a moldura geral e separadores das seções
void drawFrame() {
  tft.drawRoundRect(5, 5, 160, 310, 10, ST77XX_CYAN);  // Moldura principal
  tft.drawLine(5, 85, 165, 85, ST77XX_CYAN);           // Linhas divisórias
  tft.drawLine(5, 165, 165, 165, ST77XX_CYAN);
  tft.drawLine(5, 245, 165, 245, ST77XX_CYAN);

  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(10, 30);
  tft.print("Estacao BME280");  // Título superior
  tft.setFont();
}

// Mostra uma leitura (Temperatura, Umidade, etc.) apenas se mudar
void drawReading(const char* label, float value, const char* unit, int y, uint16_t color, float* lastValue) {
  if (abs(value - *lastValue) > 0.1) {  // Atualiza só se mudou significativamente
    tft.fillRect(10, y - 35, 120, 40, ST77XX_BLACK); // Limpa área antiga
    tft.setFont(&FreeSans9pt7b);
    tft.setCursor(10, y - 20);  // Subtítulo
    tft.setTextColor(color);
    tft.println(label);

    tft.setCursor(10, y + 6);  // Valor
    tft.setTextColor(ST77XX_WHITE);
    tft.print(value, 1);
    tft.print(" ");
    tft.print(unit);
    *lastValue = value;  // Salva valor atual
    tft.setFont();
  }
}

// Desenha ícones simples para os sensores
void drawThermometerIcon(int x, int y) {
  tft.drawLine(x, y, x, y + 12, ST77XX_RED);
  tft.fillCircle(x, y + 16, 4, ST77XX_RED);
}

void drawDropIcon(int x, int y) {
  tft.fillTriangle(x, y, x - 4, y + 8, x + 4, y + 8, ST77XX_BLUE);
  tft.fillCircle(x, y + 6, 4, ST77XX_BLUE);
}

void drawBarometerIcon(int x, int y) {
  tft.drawCircle(x, y, 8, ST77XX_YELLOW);
  tft.drawLine(x, y, x + 5, y - 3, ST77XX_YELLOW);
}

void tft_drawArc(int x, int y, int r, int start_angle, int end_angle, uint16_t color) {
  int prevX = -1, prevY = -1;
  for (float angle = start_angle; angle <= end_angle; angle += 0.5) {
    float rad = angle * 0.0174533;
    int x1 = x + r * cos(rad);
    int y1 = y + r * sin(rad);
    if (prevX >= 0) tft.drawLine(prevX, prevY, x1, y1, color);
    prevX = x1;
    prevY = y1;
  }
}
#define drawArc tft_drawArc

void drawWaveIcon(int x, int y) {
  for (int i = 0; i < 3; i++) {
    drawArc(x, y + i * 4, 6, 0, 180, ST77XX_GREEN);
  }
}

// Desenha todos os ícones
void drawIcons() {
  drawThermometerIcon(140, 40);
  drawDropIcon(140, 120);
  drawBarometerIcon(140, 200);
  drawWaveIcon(140, 280);
}

// Desenha o gráfico de pressão ao nível do mar
void drawGraph() {
  static float lastHistory[40];
  bool changed = false;
  for (int i = 0; i < 40; i++) {
    if (history[i] != lastHistory[i]) {
      changed = true;
      break;
    }
  }
  if (!changed) return;

  int baseX = 170, baseY = 315;
  int width = 140, height = 120;
  int originX = baseX;
  int originY = baseY - height;

  tft.fillRect(originX, originY, width, height, ST77XX_BLACK);
  tft.drawRect(originX, originY, width, height, ST77XX_CYAN);

  float minVal = 900, maxVal = 1100; // Faixa esperada da pressão
  for (int i = 0; i < 39; i++) {
    int x1 = originX + i * (width / 40);
    int y1 = baseY - map(history[i], minVal, maxVal, 0, height);
    int x2 = originX + (i + 1) * (width / 40);
    int y2 = baseY - map(history[i + 1], minVal, maxVal, 0, height);
    tft.drawLine(x1, y1, x2, y2, ST77XX_YELLOW);
  }

  for (int i = 0; i < 40; i++) lastHistory[i] = history[i];
}

// Inicializa tudo
void setup() {
  Serial.begin(115200);
  Wire.begin(BME_SDA, BME_SCL);
  tft.init(170, 320);
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);
  drawFrame();
  drawIcons();

  if (!bme.begin(0x76)) {
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(10, 10);
    tft.setTextSize(2);
    tft.println("BME280 N/A");
    while (true);  // Travar se não achar o sensor
  }

  for (int i = 0; i < 40; i++) history[i] = 0;
}

// Loop principal: lê sensores, mostra valores e atualiza gráfico
void loop() {
  float temp = bme.readTemperature();
  float hum  = bme.readHumidity();
  float pres = bme.readPressure() / 100.0;  // hPa
  //float pres_nmm = pres / pow(1.0 - (ALTITUDE_LOCAL / 44330.0), 5.255);  // Corrigida
  float temp_K = temp + 273.15;
  float pres_nmm = pres * pow(1.0 - (0.0065 * ALTITUDE_LOCAL) / (temp_K + 0.0065 * ALTITUDE_LOCAL), -5.257);


  // Atualiza se valores mudarem
  drawReading("Temperatura", temp, "C", 70, ST77XX_RED, &lastTemp);
  drawReading("Umidade", hum, "%", 150, ST77XX_BLUE, &lastHum);
  drawReading("Pressao", pres, "hPa", 230, ST77XX_YELLOW, &lastPres);
  drawReading("Niv. Mar", pres_nmm, "hPa", 300, ST77XX_GREEN, &lastPresNmm);

  // Atualiza histórico e gráfico
  history[historyIndex] = pres_nmm;
  historyIndex = (historyIndex + 1) % 40;
  drawGraph();

  delay(5000); // Espera 5 segundos entre leituras
}
