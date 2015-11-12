/*
Copyright (c) 2015 David Wheeler
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies
or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "DRW_SSD1306_SPI_char.h"
#include <avr/pgmspace.h>
#include <SPI.h>

// Based on a 5x7 pixel font with 1 pixel padding
// 128x64 display : 1 page = 1 row, 21 characters per row, 3 pixels unused, 1 left, 2 right

// Initialize the display
#ifdef DRW_SSD1306_SPI_USE_CS
void DRW_SSD1306_SPI_char::begin(uint8_t dc_pin, uint8_t cs_pin) {
    cs = cs_pin;
    pinMode(cs, OUTPUT);

    begin(dc_pin);
}
#endif

void DRW_SSD1306_SPI_char::begin(uint8_t dc_pin) {
    dc = dc_pin;

    char_cursor_x = 0x00;
    char_cursor_y = 0x00;

    pinMode(dc, OUTPUT);

#ifdef DRW_SSD1306_SPI_USE_CS
    digitalWrite(cs, LOW);
#endif

    digitalWrite(dc, LOW);

    SPI.transfer(SSD1306_DISPLAYOFF);                    // 0xAE : Turn the display off

    // Clear the GDDRAM
    SPI.transfer(SSD1306_SETLOWCOLUMN);                  // 0x00 : Set current GDDRAM column address
    SPI.transfer(SSD1306_SETHIGHCOLUMN);                 // 0x10 : to 0x00
	SPI.transfer(SSD1306_PAGESTART);                     // 0x80 : Set current GDDRAM page to 0x00

    digitalWrite(dc, HIGH);

    const uint8_t zero = 0x00;

    for (uint16_t i = 1025; i > 0; i--) SPI.transfer(zero); // GDDRAM cursor will have looped back to 0, 0 so 1025 adds the left margin pixel

    digitalWrite(dc, LOW);

    uint8_t commands[] = {
        SSD1306_SETMULTIPLEX, 0x3f,       // 0xA8 : Set the multiplex mode for 64 row display
        SSD1306_SETDISPLAYOFFSET, 0x00,   // 0xD3 : Set the vertical RAM to display offset to 00h
        SSD1306_SETSTARTLINE | 0x0,       // 0x40 : Set the start RAM start line to physical line 0
        SSD1306_MEMORYMODE, 0x00,         // 0x20 : Set memory mode to horizontal
        SSD1306_SEGREMAP | 0x1,           // 0xA1 : Mirror the segments
        SSD1306_COMSCANDEC,               // 0xC8 : Set the COM scan direction decreaasing (0,0 is now topleft)
        SSD1306_SETPRECHARGE, 0xf1,       // 0xD9 : Set the pixel pre-charge period
        SSD1306_SETVCOMDETECT, 0x40,      // 0xDB : Set the Vcom deselect voltage
        SSD1306_SETCOMPINS, 0x12,         // 0xDA : Set COM pins configuration
        SSD1306_SETCONTRAST, 0xcf,        // 0x81 : Set display contrast
        SSD1306_DISPLAYALLON_RESUME,      // 0xA4 : Display RAM contents
        SSD1306_NORMALDISPLAY,            // 0xA6 : Set display to normal ( non-inverted )
        SSD1306_SETDISPLAYCLOCKDIV, 0x80, // 0xD5 : Set the display clock frequency ( 370KHz ? ) and divide ratio
        SSD1306_CHARGEPUMP, 0x14,         // 0x8D : Turn the OLED driver voltage charge pump
        SSD1306_DISPLAYON                 // 0xAF : Turn the display on
    };

    SPI.transfer(commands, sizeof(commands));

#ifdef DRW_SSD1306_SPI_USE_CS
    if (cs >= 0) digitalWrite(cs, HIGH);
#endif
}

// Clears the whole display and sets the character cursor to 0, 0
void DRW_SSD1306_SPI_char::clear_display() {
#ifdef DRW_SSD1306_SPI_USE_CS
    if (cs >= 0) digitalWrite(cs, LOW);
#endif

    digitalWrite(dc, LOW);

    // Set GDDRAM cursor to 0, 0
    SPI.transfer(SSD1306_SETLOWCOLUMN);
    SPI.transfer(SSD1306_SETHIGHCOLUMN);
	SPI.transfer(SSD1306_PAGESTART);

    // Send 1025 0x00 to clear the whole GDDRAM and add the left padding
    digitalWrite(dc, HIGH);

    const uint8_t zero = 0x00;

    for (uint16_t i = 1025; i > 0; i--) SPI.transfer(zero);

#ifdef DRW_SSD1306_SPI_USE_CS
    if (cs >= 0) digitalWrite(cs, HIGH);
#endif
}

// Sets the character cursor position
// 5x7 font with 1 pixel left margin, 5 font pixel and 1 padding pixel, 1 pixel right margin
// x from 0 to 20
// y from 0 to 7
void DRW_SSD1306_SPI_char::set_char_cursor(uint8_t x, uint8_t y) {
    if (x > 20 || y > 7) return;

    uint8_t pixel_x = 1 + (6 * x); // 5x7 font with 1 pixel left margin, 5 font pixel and 1 padding pixel (and 2 pixel right margin)

#ifdef DRW_SSD1306_SPI_USE_CS
    if (cs >= 0) digitalWrite(cs, LOW);
#endif

    digitalWrite(dc, LOW);

    SPI.transfer(SSD1306_SETLOWCOLUMN  +  (pixel_x       & 0x0F));
    SPI.transfer(SSD1306_SETHIGHCOLUMN + ((pixel_x >> 4) & 0x0F));
	SPI.transfer(SSD1306_PAGESTART     +   y);

#ifdef DRW_SSD1306_SPI_USE_CS
    if (cs >= 0) digitalWrite(cs, HIGH);
#endif

    char_cursor_x = x;
    char_cursor_y = y;
}

uint8_t DRW_SSD1306_SPI_char::get_char_cursor_x() {
    return char_cursor_x;
}

uint8_t DRW_SSD1306_SPI_char::get_char_cursor_y() {
    return char_cursor_y;
}

// Print low level character write
// Based on a 5x7 pixel font with 1 pixel padding
// 128x64 display : 1 page = 1 row, 21 characters per row, 3 pixels unused, 1 left, 2 right
size_t DRW_SSD1306_SPI_char::write(uint8_t c) {
    if (c == '\r') { // Carriage return - move to beginning of current line
        set_char_cursor(0, char_cursor_y);
        return 1;
    }

    if (c == '\n') { // New line - move down a line
        set_char_cursor(char_cursor_x, (char_cursor_y + 1) & 0x7);
        return 1;
    }

    if (c < 32 || c > 127) { // Print characters not in the font as <SPACE>
		c = 32;
    }

#ifdef DRW_SSD1306_SPI_USE_CS
    if (cs >= 0) digitalWrite(cs, LOW);
#endif

    digitalWrite(dc, HIGH);

    for (byte i = 0; i < 5; i++) { // Send the character, 1 column at a time
        SPI.transfer(pgm_read_byte(&font5x7[c - 32][i]));
    }

    const uint8_t zero = 0x00;

    SPI.transfer(zero); // Right padding pixel

    char_cursor_x++; // Move right one character to catch up with the hardware cursor position

    if (char_cursor_x > 20) {
        SPI.transfer(zero); // Right margin pixel and moves the GDDRAM cursor to the start of the next line
        SPI.transfer(zero); // Left margin pixel

        char_cursor_x = 0;
        char_cursor_y = (char_cursor_y + 1) & 0x7;
    }

#ifdef DRW_SSD1306_SPI_USE_CS
    if (cs >= 0) digitalWrite(cs, HIGH);
#endif

    return 1;
}

// Prints a string
size_t DRW_SSD1306_SPI_char::write(const uint8_t *buffer, size_t size) {
    size_t  n = 0;
    uint8_t c;

    while (size--) {
        c = *buffer++;

        if (c == '\r') { // Carriage return - move to beginning of current line
            set_char_cursor(0, char_cursor_y);
            n++;
            continue;
        }

        if (c == '\n') { // New line - move down a line
            set_char_cursor(char_cursor_x, (char_cursor_y + 1) & 0x7);
            n++;
            continue;
        }

        if (c < 32 || c > 127) { // Print characters not in the font as <SPACE>
            c = 32;
        }

#ifdef DRW_SSD1306_SPI_USE_CS
        if (cs >= 0) digitalWrite(cs, LOW);
#endif

        digitalWrite(dc, HIGH); // Start writing to the GDDRAM

        for (byte i = 0; i < 5; i++) { // Send the character, 1 column at a time
            SPI.transfer(pgm_read_byte(&font5x7[c - 32][i]));
        }

        const uint8_t zero = 0x00;

        SPI.transfer(zero); // Right padding pixel

        char_cursor_x++; // Move right one character to catch up with the hardware cursor position
        n++;

        if (char_cursor_x > 20) {
            SPI.transfer(zero); // Right margin pixel and moves the GDDRAM cursor to the start of the next line
            SPI.transfer(zero); // Left margin pixel

            char_cursor_x = 0;
            char_cursor_y = (char_cursor_y + 1) & 0x7;
        }
    }

#ifdef DRW_SSD1306_SPI_USE_CS
    if (cs >= 0) digitalWrite(cs, HIGH);
#endif

    return n;
}
