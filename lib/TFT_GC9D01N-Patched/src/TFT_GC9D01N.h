
#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <SPI.h>

#define LANDSCAPE 1   //Horizontal screen
//#define PORTRAIT 2
#define CHAR_FONT_W8_H16  //typeface
#ifndef TFT_WIDTH
#define TFT_WIDTH  40
#endif
#ifndef TFT_HEIGHT
#define TFT_HEIGHT 160
#endif

#ifndef TFT_MISO
#define TFT_MISO  -1
#endif

#ifndef TFT_MOSI
#define TFT_MOSI  10//21
#endif

#ifndef TFT_SCLK
#define TFT_SCLK  20//22
#endif
#ifndef TFT_CS
#define TFT_CS    -1//18  // Chip select control pin
#endif
#ifndef TFT_DC
#define TFT_DC    2  // Data Command control pin
#endif
#ifndef TFT_RST
#define TFT_RST   -1//27
#endif
#ifndef TFT_BL
#define TFT_BL    8//4
#endif

/*
#define TFT_MISO  -1
#define TFT_MOSI  21
#define TFT_SCLK  22
#define TFT_CS    18  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   27
#define TFT_BL    23//4
*/
//RGB565
#define RED    0xF800
#define GREEN  0x07E0
#define BLUE   0x001F
#define WHITE  0xFFFF
#define BLACK  0x0000
#define GRAY   0xEF5D
#define GRAY75 0x39E7
#define GRAY50 0x7BEF
#define GRAY25 0xADB5



#ifdef  CHAR_FONT_W8_H16
#define  FONT_W  8
#define  FONT_H  16
#endif


#define SPI_FREQUENCY  40000000
#define DC_C digitalWrite(TFT_DC, LOW)
#define DC_D digitalWrite(TFT_DC, HIGH)
#if (TFT_CS==-1)
#define CS_L ;//digitalWrite(TFT_CS, LOW)
#define CS_H ;//digitalWrite(TFT_CS, HIGH)
#else
#define CS_L digitalWrite(TFT_CS, LOW)
#define CS_H digitalWrite(TFT_CS, HIGH)
#endif

#if (TFT_RST==-1)
#define RST_L ;//digitalWrite(TFT_RST, LOW)
#define RST_H ;//digitalWrite(TFT_RST, HIGH)
#else
#define RST_L digitalWrite(TFT_RST, LOW)
#define RST_H digitalWrite(TFT_RST, HIGH)
#endif
typedef const uint8_t* BitArray;

class TFT_GC9D01N_Class
{
public:

    int begin();
    void begin_tft_write();
    void end_tft_write();
    void writedata(uint8_t d);
    void writecommand(uint8_t c);
    void lcd_init();
    void backlight(uint8_t state);
    void BlockWrite(unsigned int Xstart, unsigned int Xend, unsigned int Ystart, unsigned int Yend) ;
    void DrawImage(unsigned int Xstart, unsigned int Ystart, unsigned int Xend, unsigned int Yend, uint16_t *color);
    void DrawImage(unsigned int Xstart, unsigned int Ystart, unsigned int w, unsigned int h,const uint8_t *buffer, uint16_t fgcolor, uint16_t bgcolor);
//    void DrawBitImage(unsigned int Xstart, unsigned int Ystart, unsigned int w, unsigned int h, BitArray buffer, uint16_t fgcolor, uint16_t bgcolor);
    void DrawBitImage(unsigned int Xstart, unsigned int Ystart, unsigned int w, unsigned int h, BitArray buffer, uint16_t fgcolor, uint16_t bgcolor);

    void DispColor(unsigned int Xstart, unsigned int Ystart, unsigned int Xend, unsigned int Yend, unsigned int color);
    void WriteOneDot(unsigned int color);
    void  DispOneChar( char ord, unsigned int Xstart, unsigned int Ystart, unsigned int TextColor, unsigned int BackColor);
    void DispStr( char *str, unsigned int Xstart, unsigned int Ystart, unsigned int TextColor, unsigned int BackColor);

};
