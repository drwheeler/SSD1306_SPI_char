# SSD1306_SPI_char
SSD1306 128x64 SPI display used as a 21x8 character display for Arduino

Timings for a 4MHz SPI clock:

begin      : 4.804ms - set everything up and clear the GDDRAM

set cursor : 39.17us - set the GDDRAM pointer to a new position

write      : 61.21us - write the 6 bytes of a single 5x7 character plus a padding byte
