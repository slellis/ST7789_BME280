# ST7789_BME280

Este projeto utiliza um **ESP32**, um display **ST7789 SPI 1.9" (170x320)** e um sensor **BME280** para exibir, em tempo real, os seguintes dados meteorológicos:

- Temperatura (°C)
- Umidade relativa (%)
- Pressão atmosférica local (hPa)
- Pressão ao nível do mar (hPa)
- Gráfico de tendência de pressão (últimas 40 amostras)

## 🛠️ Componentes usados

| Componente      | Descrição                                     |
|-----------------|-----------------------------------------------|
| ESP32 (38 pinos)| Microcontrolador principal                    |
| Display ST7789  | SPI, 1.9", resolução 170x320                  |
| Sensor BME280   | I2C, leitura de temperatura, umidade, pressão |
| Jumpers, fonte  | Alimentação e conexões básicas                |

## 🔌 Ligações

### ST7789

| Display | ESP32 GPIO |
|--------|-------------|
| CS     | 5           |
| DC     | 19          |
| RST    | 4           |
| SCLK   | 18          |
| MOSI   | 23          |
| VCC    | 3.3V        |
| GND    | GND         |

### BME280 (I2C)

| BME280 | ESP32 GPIO |
|--------|-------------|
| SDA    | 21          |
| SCL    | 22          |
| VCC    | 3.3V        |
| GND    | GND         |

## 💾 Código

O arquivo principal do projeto é:

```
ST7789_BME280.ino
```

Este sketch realiza:

- Inicialização do display com moldura e ícones gráficos
- Leitura periódica do BME280
- Cálculo da pressão ao nível do mar com base em `ALTITUDE_LOCAL`
- Exibição somente de valores alterados para minimizar **flicker**
- Desenho de gráfico com histórico de pressão

## 📷 Preview

> *(Insira aqui uma imagem do display em funcionamento, se desejar)*

## 📦 Bibliotecas necessárias

Instale as seguintes bibliotecas na Arduino IDE:

- `Adafruit GFX Library`
- `Adafruit ST7789 Library`
- `Adafruit BME280 Library`
- `Adafruit Unified Sensor`

## 📐 Ajustes

- Se sua altitude local for diferente de **1040 m**, altere a constante `ALTITUDE_LOCAL` no código.
- O display deve estar na **orientação vertical** (rotation 0).

## 📅 Atualizações

- A cada 5 segundos, os dados são atualizados.
- Gráfico armazena até **40 leituras** (~3 minutos e 20 segundos de histórico).

## 📜 Licença

Este projeto é open-source. Sinta-se livre para estudar, adaptar e compartilhar.