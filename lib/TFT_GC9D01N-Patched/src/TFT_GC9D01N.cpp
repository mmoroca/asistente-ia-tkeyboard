

#include "TFT_GC9D01N.h"
#include "Fonts.h"

void  TFT_GC9D01N_Class::begin_tft_write()
{
    SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0)); // RP2040 SDK -> 68us delay!
    CS_L;
    //  SET_BUS_WRITE_MODE;
}

void TFT_GC9D01N_Class::end_tft_write()
{
    // SPI_BUSY_CHECK;       // Check send complete and clean out unused rx data
    CS_H;
    SPI.endTransaction(); //  RP2040 SDK -> 0.7us delay
}

void TFT_GC9D01N_Class::writedata(uint8_t d)
{
    begin_tft_write();

    DC_D;        // Play safe, but should already be in data mode  高电平1为数据

    SPI.transfer(d);

    CS_L;        // Allow more hold time for low VDI rail

    end_tft_write();
}

void TFT_GC9D01N_Class::writecommand(uint8_t c)
{
    begin_tft_write();

    DC_C;       //高电平1为数据

    SPI.transfer(c);//tft_Write_8(c);

    DC_D;

    end_tft_write();

}

void TFT_GC9D01N_Class::lcd_init()
{

    RST_H;
    delay(80);

    RST_L;
    delay(80);

    RST_H;
    delay(480);


    writecommand(0xFE);
    writecommand(0xEF);

    writecommand(0x80);
    writedata(0xFF);

    writecommand(0x81);
    writedata(0xFF);

    writecommand(0x82);
    writedata(0xFF);

    writecommand(0x83);
    writedata(0xFF);

    writecommand(0x84);
    writedata(0xFF);

    writecommand(0x85);
    writedata(0xFF);

    writecommand(0x86);
    writedata(0xFF);

    writecommand(0x87);
    writedata(0xFF);

    writecommand(0x88);
    writedata(0xFF);

    writecommand(0x89);
    writedata(0xFF);

    writecommand(0x8A);
    writedata(0xFF);

    writecommand(0x8B);
    writedata(0xFF);

    writecommand(0x8C);
    writedata(0xFF);

    writecommand(0x8D);
    writedata(0xFF);

    writecommand(0x8E);
    writedata(0xFF);

    writecommand(0x8F);
    writedata(0xFF);

    writecommand(0x3A);
    writedata(0x05);

    writecommand(0xEC);
    writedata(0x11);


    writecommand(0x7E);
    writedata(0x7a);


    writecommand(0x74);
    writedata(0x02);
    writedata(0x0E);
    writedata(0x00);
    writedata(0x00);
    writedata(0x28);
    writedata(0x00);
    writedata(0x00);

    writecommand(0x98);
    writedata(0x3E);
    writecommand(0x99);
    writedata(0x3E);




    writecommand(0xB5);
    writedata(0x0E);
    writedata(0x0E);
    writecommand(0x60);
    writedata(0x38);
    writedata(0x09);
    writedata(0x6D);
    writedata(0x67);


    writecommand(0x63);
    writedata(0x38);
    writedata(0xAD);
    writedata(0x6D);
    writedata(0x67);
    writedata(0x05);


    writecommand(0x64);
    writedata(0x38);
    writedata(0x0B);
    writedata(0x70);
    writedata(0xAB);
    writedata(0x6D);
    writedata(0x67);


    writecommand(0x66);
    writedata(0x38);
    writedata(0x0F);
    writedata(0x70);
    writedata(0xAF);
    writedata(0x6d);
    writedata(0x67);

    writecommand(0x6A);
    writedata(0x00);
    writedata(0x00);

    writecommand(0x68);
    writedata(0x3B);
    writedata(0x08);
    writedata(0x04);
    writedata(0x00);
    writedata(0x04);
    writedata(0x64);
    writedata(0x67);



    writecommand(0x6C);
    writedata(0x22);
    writedata(0x02);
    writedata(0x22);
    writedata(0x02);
    writedata(0x22);
    writedata(0x22);
    writedata(0x50);

    writecommand(0x6E);
    writedata(0x00);
    writedata(0x00);
    writedata(0x00);
    writedata(0x00);
    writedata(0x07);
    writedata(0x01);
    writedata(0x13);
    writedata(0x11);

    writedata(0x0B);
    writedata(0x09);
    writedata(0x16);
    writedata(0x15);
    writedata(0x1D);
    writedata(0x1E);
    writedata(0x00);
    writedata(0x00);

    writedata(0x00);
    writedata(0x00);
    writedata(0x1E);

    writedata(0x1D);
    writedata(0x15);
    writedata(0x16);
    writedata(0x0A);
    writedata(0x0C);

    writedata(0x12);
    writedata(0x14);
    writedata(0x02);
    writedata(0x08);
    writedata(0x00);
    writedata(0x00);
    writedata(0x00);
    writedata(0x00);


    writecommand(0xA9);
    writedata(0x1B);

    writecommand(0xA8);
    writedata(0x6B);

    writecommand(0xA8);
    writedata(0x6D);

    writecommand(0xA7);
    writedata(0x40);

    writecommand(0xAD);
    writedata(0x47);


    writecommand(0xAF);
    writedata(0x73);

    writecommand(0xAF);
    writedata(0x73);

    writecommand(0xAC);
    writedata(0x44);

    writecommand(0xA3);
    writedata(0x6C);

    writecommand(0xCB);
    writedata(0x00);

    writecommand(0xCD);
    writedata(0x22);


    writecommand(0xC2);
    writedata(0x10);

    writecommand(0xC5);
    writedata(0x00);

    writecommand(0xC6);
    writedata(0x0E);

    writecommand(0xC7);
    writedata(0x1f);

    writecommand(0xC8);
    writedata(0x0E);

    writecommand(0xbf);
    writedata(0x00);

    writecommand(0xF9);
    writedata(0x20);

    writecommand(0x9b);
    writedata(0x3b);
    writecommand(0x93);
    writedata(0x33);
    writedata(0x7f);
    writedata(0x00);

    writecommand(0x70);
    writedata(0x0E);
    writedata(0x0f);
    writedata(0x03);
    writedata(0x0e);
    writedata(0x0f);
    writedata(0x03);

    writecommand(0x71);
    writedata(0x0e);
    writedata(0x16);
    writedata(0x03);

    writecommand(0x91);
    writedata(0x0e);
    writedata(0x09);



    writecommand(0xc3);
    writedata(0x2c);
    writecommand(0xc4);
    writedata(0x1a);




    writecommand(0xf0);
    writedata(0x51);
    writedata(0x13);
    writedata(0x0c);
    writedata(0x06);
    writedata(0x00);
    writedata(0x2f);

    writecommand(0xf2);
    writedata(0x51);
    writedata(0x13);
    writedata(0x0c);
    writedata(0x06);
    writedata(0x00);
    writedata(0x33);

    writecommand(0xf1);
    writedata(0x3c);
    writedata(0x94);
    writedata(0x4f);
    writedata(0x33);
    writedata(0x34);
    writedata(0xCf);

    writecommand(0xf3);
    writedata(0x4d);
    writedata(0x94);
    writedata(0x4f);
    writedata(0x33);
    writedata(0x34);
    writedata(0xCf);


#define TFT_MAD_MY  0x80
#define TFT_MAD_MX  0x40
#define TFT_MAD_MV  0x20
#define TFT_MAD_ML  0x10

    writecommand(0x36);

//#if LANDSCAPE
//writedata(0x00 |  TFT_MAD_MX | TFT_MAD_MY);

#if (LANDSCAPE==2)||(PORTRAIT==2)
    writedata(0x00);
#else
    writedata(0x00 |  TFT_MAD_MX | TFT_MAD_MY);
#endif
    writecommand(0x11);
    delay(200);
    writecommand(0x29);
    writecommand(0x2C);


}

//init lcd
int TFT_GC9D01N_Class::begin()
{

    pinMode(TFT_BL, OUTPUT);
    pinMode(TFT_SCLK, OUTPUT);
    pinMode(TFT_MISO, OUTPUT);
    pinMode(TFT_MOSI, OUTPUT);
    pinMode(TFT_DC, OUTPUT);

    if (TFT_CS >= 0) pinMode(TFT_CS, OUTPUT);   // -1: pin no usado
    if (TFT_RST >= 0) pinMode(TFT_RST, OUTPUT);

    ledcSetup(1, 1000, 10);
    ledcAttachPin(TFT_BL, 1);
    ledcWrite(1, 0);

    delay(100);
    SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

    // Reset por software del controlador. TFT_RST no está conectado (-1), así que un reinicio del
    // ESP32 no reinicia la pantalla: sin esto, la pantalla conserva su estado y sale gris o borrosa.
    writecommand(0x01);
    delay(150);

    int i = 2;
    while (i--) {
        lcd_init();
        delay(200); /* code */
    }
    ledcWrite(1, 220);//开启背光

    return 1;

}

void TFT_GC9D01N_Class::backlight(uint8_t state)
{
    ledcWrite(1, state);// digitalWrite(TFT_BL, state);
}

/**
 * @brief   Select the block area to write to 选择写的块区域
 * @note
 * @param  Xstart:
 * @param  Xend:
 * @param  Ystart:
 * @param  Yend:
 * @retval None
 */
void TFT_GC9D01N_Class::BlockWrite(unsigned int Xstart, unsigned int Xend, unsigned int Ystart, unsigned int Yend)
{

    writecommand(0x2A);
    writedata(Xstart >> 8);
    writedata(Xstart);
    writedata(Xend >> 8);
    writedata(Xend);

    writecommand(0x2B);
    writedata(Ystart >> 8);
    writedata(Ystart);
    writedata(Yend >> 8);
    writedata(Yend);

    writecommand(0x2c);

}

/**
 * @brief Drawing function
 * @note
 * @param  Xstart:  starting x-coordinate x坐标开始
 * @param  Ystart:  starting y-coordinate y坐标结束
 * @param  w:       The width of the image
 * @param  h:       The height of the image
 * @param  color:
 * @retval None
 */
void TFT_GC9D01N_Class::DrawImage(unsigned int Xstart, unsigned int Ystart, unsigned int w, unsigned int h, uint16_t *color)
{
    unsigned int i, j;
    unsigned int Xend = Xstart + w;
    unsigned int Yend = Ystart + h;

    BlockWrite(Xstart, Xend - 1, Ystart, Yend - 1);
    uint32_t len = w * h;


#if LANDSCAPE

    SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
    CS_L;
    DC_D;
    for (int i = 0; i <= h; i++) {
        for (int j = w - 1 ; j >= 0 ; j--) {
            SPI.transfer16(*(color + ((j * (Yend - Ystart)) + (i))));
        }
    }
    CS_H;
    SPI.endTransaction();

#else

    SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
    CS_L;
    DC_D;
    for (int i = 0; i <= h; i++) {
        for (int j = w - 1 ; j >= 0 ; j--) {
            SPI.transfer16(*color);
            color++;
        }
    }
    CS_H;
    SPI.endTransaction();

#endif

}


void TFT_GC9D01N_Class::DispColor(unsigned int Xstart, unsigned int Ystart, unsigned int Xend, unsigned int Yend, unsigned int color)
{


    unsigned int i, j;
    uint32_t len = TFT_HEIGHT * TFT_WIDTH;
    BlockWrite(Xstart, Xend - 1, Ystart, Yend - 1);

    SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
    CS_L;
    DC_D;
    for (i = 0; i <= TFT_HEIGHT; i++) {
        for (j = 0; j < TFT_WIDTH; j++) {
            SPI.transfer16(color);
        }
    }
    CS_H;
    SPI.endTransaction();

}

void TFT_GC9D01N_Class::WriteOneDot(unsigned int color)
{


    CS_L;
    DC_D;

    writedata(color >> 8);
    writedata(color);

    CS_H;

}

unsigned char ToOrd( char ch)
{
    if (ch < 32) {
        ch = 95;
    } else if ((ch >= 32) && (ch <= 47)) {
        ch = (ch - 32) + 10 + 62;
    } else if ((ch >= 48) && (ch <= 57)) {
        ch = ch - 48;
    } else if ((ch >= 58) && (ch <= 64)) {
        ch = (ch - 58) + 10 + 62 + 16;
    } else if ((ch >= 65) && (ch <= 126)) {
        ch = (ch - 65) + 10;
    } else if (ch > 126) {
        ch = 95;
    }

    return ch;
}

// Maps a 2-byte UTF-8 sequence (lead byte 0xC2/0xC3 + continuation byte)
// to an extra glyph index (96..111). Unknown sequences map to the block glyph (95).
unsigned char Utf8ToOrd(unsigned char lead, unsigned char cont)
{
    if (lead == 0xC3) {
        switch (cont) {
            case 0xA1: return 96;   // a with acute
            case 0xA9: return 97;   // e with acute
            case 0xAD: return 98;   // i with acute
            case 0xB3: return 99;   // o with acute
            case 0xBA: return 100;  // u with acute
            case 0xBC: return 101;  // u with diaeresis
            case 0xB1: return 102;  // n with tilde
            case 0x81: return 103;  // A with acute
            case 0x89: return 104;  // E with acute
            case 0x8D: return 105;  // I with acute
            case 0x93: return 106;  // O with acute
            case 0x9A: return 107;  // U with acute
            case 0x9C: return 108;  // U with diaeresis
            case 0x91: return 109;  // N with tilde
        }
    } else if (lead == 0xC2) {
        switch (cont) {
            case 0xBF: return 110;  // inverted question mark
            case 0xA1: return 111;  // inverted exclamation mark
        }
    }
    return 95;
}

void hz816_s90(const unsigned char *p_ucHz, unsigned char *p_ucResult) //A conversion function that rotates an 8x16 character font 90 degrees clockwise 把8x16字符字模顺时针旋转90度的转换函数
{

    unsigned char a;

    unsigned char b;

    unsigned char c;

    unsigned char uiBuffer[16];
    for (a = 0; a < 16; a++)

    {
        uiBuffer[a] = p_ucHz[a];
    }

    c = 0;

    for (a = 0; a < 8; a++)

    {

        for (b = 0; b < 8; b++)

        {

            p_ucResult[c] = p_ucResult[c] << 1;

            p_ucResult[c] = p_ucResult[c] & 0xfe;

            if (uiBuffer[15 - b] >= 0x80)

            {

                p_ucResult[c] = p_ucResult[c] + 1;

            }

            uiBuffer[15 - b] = uiBuffer[15 - b] << 1;


        }

        c++;

        for (b = 0; b < 8; b++)

        {

            p_ucResult[c] = p_ucResult[c] << 1;

            p_ucResult[c] = p_ucResult[c] & 0xfe;

            if (uiBuffer[7 - b] >= 0x80)

            {

                p_ucResult[c] = p_ucResult[c] + 1;

            }

            uiBuffer[7 - b] = uiBuffer[7 - b] << 1;

        }

        c++;

    }

}

void hz816_s270(const unsigned char *p_ucHz, unsigned char *p_ucResult)
{

    unsigned char a;

    unsigned char b;

    unsigned char c;

    unsigned char uiBuffer[16];


    for (a = 0; a < 16; a++)

    {

        uiBuffer[a] = p_ucHz[a];

    }

    c = 0;

    for (a = 0; a < 8; a++)

    {

        for (b = 0; b < 8; b++)

        {

            p_ucResult[c] = p_ucResult[c] << 1;


            if ((uiBuffer[b] & 0x01)  >= 0x01)

            {

                p_ucResult[c] = p_ucResult[c] + 1;

            }

            uiBuffer[b] = uiBuffer[b] >> 1;


        }

        c++;

        for (b = 0; b < 8; b++)

        {

            p_ucResult[c] = p_ucResult[c] << 1;

            if ((uiBuffer[8 + b] & 0x01) >= 0x01)

            {

                p_ucResult[c] = p_ucResult[c] + 1;

            }

            uiBuffer[8 + b] = uiBuffer[8 + b] >> 1;

        }

        c++;

    }

}

void  TFT_GC9D01N_Class::DispOneChar( char ord, unsigned int Xstart, unsigned int Ystart, unsigned int TextColor, unsigned int BackColor) // ord:0~95
{
    unsigned char  hou[16] = {0};
    unsigned char i, j;
    unsigned char  *p;
    unsigned char dat;
    unsigned int index;


    index = ord;

    if (index >= FONT_GLYPH_COUNT)
        index = 95;

    index = index * ((FONT_W / 8) * FONT_H);

    p = ascii;
    p = p + index ;


#if LANDSCAPE

    int y = Ystart;
    y = y - 23;
    Ystart = abs(y);


    BlockWrite(Ystart, Ystart + (FONT_H - 1), Xstart, Xstart + (FONT_W - 1));

    hz816_s90(p, hou);

    for (i = 0; i < (FONT_W / 8 * FONT_H); i++) {
        dat = hou[i];
        for (j = 0; j < 8; j++) {
            if ((dat << j) & 0x80) {
                WriteOneDot(TextColor);
            } else {
                WriteOneDot(BackColor);
            }
        }
    }
#else
    BlockWrite(Xstart, Xstart + (FONT_W - 1), Ystart, Ystart + (FONT_H - 1));


    for (i = 0; i < (FONT_W / 8 * FONT_H); i++) {
        dat = *p++;
        for (j = 0; j < 8; j++) {
            if ((dat << j) & 0x80) {
                WriteOneDot(TextColor);
            } else {
                WriteOneDot(BackColor);
            }
        }
    }

#endif
}

void TFT_GC9D01N_Class::DispStr( char *str, unsigned int Xstart, unsigned int Ystart, unsigned int TextColor, unsigned int BackColor)
{

    while (!(*str == '\0')) {
        unsigned char c = (unsigned char)*str++;
        unsigned char ord;
        if ((c == 0xC2 || c == 0xC3) && *str != '\0') {
            unsigned char c2 = (unsigned char)*str++;
            ord = Utf8ToOrd(c, c2);
        } else {
            ord = ToOrd((char)c);
        }
        DispOneChar((char)ord, Xstart, Ystart, TextColor, BackColor);

#if LANDSCAPE
        if (Xstart > ((TFT_HEIGHT - 1) - FONT_W)) {
            //Xstart = 0;
            //Ystart = Ystart - FONT_H;
        } else {
            Xstart = Xstart + FONT_W;
        }

        if (Ystart > ((TFT_WIDTH - 1) - FONT_H)) {
            Ystart = 23;
        }
#else


        if (Xstart > ((TFT_WIDTH - 1) - FONT_W)) {
            Xstart = 0;
            Ystart = Ystart + FONT_H;
        } else {
            Xstart = Xstart + FONT_W;
        }

        if (Ystart > ((TFT_HEIGHT - 1) - FONT_H)) {
            Ystart = 0;
        }

#endif


    }
    BlockWrite(0, TFT_WIDTH - 1, 0, TFT_HEIGHT - 1);
}

/*
 * @brief   Draw a bitmap image from a gray scale buffer mapped between two colours
 */
void
TFT_GC9D01N_Class::DrawImage(unsigned int Xstart, unsigned int Ystart, unsigned int w, unsigned int h, const uint8_t *buffer,
                             uint16_t fgcolor, uint16_t bgcolor) {


    uint16_t fg_b, fg_g, fg_r, bg_b, bg_g, bg_r;
    fg_b = (fgcolor & 0x001F) << 3;
    fg_g = (fgcolor & 0x07E0) >> 3;
    fg_r = (fgcolor & 0xF800) >> 8;
    bg_b = (bgcolor & 0x001F) << 3;
    bg_g = (bgcolor & 0x07E0) >> 3;
    bg_r = (bgcolor & 0xF800) >> 8;
    // Set the block area to write to
    BlockWrite(Xstart, Xstart + h - 1, Ystart, Ystart + w - 1); // Adjusted for landscape orientation


    SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));CS_L;
    DC_D;
#if LANDSCAPE
    for (uint col = 0; col < w; col++) {
        for (int row = h - 1; row >= 0; row--) {
            // Calculate the bit index for the rotated image
            uint8_t gray = buffer[(row * w + col)];

            uint16_t color = ((((fg_r - bg_r)) * gray) / 255 + bg_r) << 11; // red
            color |= (((fg_g - bg_g)) * gray / 255 + bg_g) << 5; // green
            color |= (((fg_b - bg_b)) * gray / 255 + bg_b);      // blue
            SPI.transfer16(color);
        }
    }


#else


    for (uint row = 0; row < h; row++) {
        for (uint col = 0; col < w; col++) {
            uint8_t gray = buffer[(row * w + col)];

            uint16_t color = ((((fg_r - bg_r)) * gray) / 255 + bg_r) << 11; // red
            color |= (((fg_g - bg_g)) * gray / 255 + bg_g) << 5; // green
            color |= (((fg_b - bg_b)) * gray / 255 + bg_b);      // blue
            SPI.transfer16(color);
        }
    }

#endif
    CS_H;
    SPI.endTransaction();


}

void TFT_GC9D01N_Class::DrawBitImage(unsigned int Xstart, unsigned int Ystart, unsigned int w, unsigned int h,
                                      BitArray buffer, uint16_t fgcolor, uint16_t bgcolor) {
    // Set the block area to write to
    BlockWrite(Xstart, Xstart + h - 1, Ystart, Ystart + w - 1); // Adjusted for landscape orientation


    SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));CS_L;
    DC_D;
#if LANDSCAPE
    for (uint col = 0; col < w; col++) {
        for (int row = h - 1; row >= 0; row--) {
            // Calculate the bit index for the rotated image
            uint8_t bit = buffer[(row * w + col) / 8] & (1 << (7 - ((row * w + col) % 8)));
            uint16_t color = bit ? fgcolor : bgcolor;
            SPI.transfer16(color);
        }
    }


#else


    for (uint row = 0; row < h; row++) {
        for (uint col = 0; col < w; col++) {
            // Calculate the bit index for landscape orientation
            uint8_t bit = buffer[(row * w + col) / 8] & (1 << (7 - ((row * w + col) % 8)));
            uint16_t color = bit ? fgcolor : bgcolor;
            SPI.transfer16(color);
        }
    }

#endif
    CS_H;
    SPI.endTransaction();

}