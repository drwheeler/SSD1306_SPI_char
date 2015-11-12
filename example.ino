#include <SPI.h>
#include <DRW_SSD1306_SPI_char.h>

const uint8_t cs_pin = 2;
const uint8_t dc_pin = 1;

DRW_SSD1306_SPI_char oled;

void setup() {
  Serial.begin();
  SPI.begin();
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0)); // 4MHz clock
  oled.begin(dc_pin, cs_pin);
  oled.write("Hello, world!\r\n");
}

void loop() {
  static uint8_t x = 0;
  static uint8_t y = 0;
  static uint8_t n = 0;
  x = random(0, 21);
  y = random(0, 8);
  n = random(0, 9);

  oled.set_char_cursor(x, y);
  oled.print(n);
}
