// SSD1306 (partially) rewritten for trinket

#define I2C_ADDR 0x3C

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// render pixels
// (i want 2x2, and the trinket doesn't have enough memory for the
// full screen)
#define P_WIDTH (OLED_WIDTH / 2)
#define P_HEIGHT (OLED_HEIGHT / 2)

#define WIRE_BUFFER_SIZE 16

// i want 2x2 pixels, so half the dimensions, and the 1 bit per pixel
// +7/8 is to get ceil
byte buf[P_WIDTH * ((P_HEIGHT + 7) / 8)];

#define GET_PIXEL(x, y) (buf[(x) + ((y) / 8) * SCREEN_WIDTH] & (1 << ((y) & 7)))

/*** REGISTER DEFINITIONS, ETC ***/

#define BLACK               0    ///< Draw 'off' pixels
#define WHITE               1    ///< Draw 'on' pixels
#define INVERSE             2    ///< Invert pixels

#define MEMORYMODE          0x20 ///< See datasheet
#define COLUMNADDR          0x21 ///< See datasheet
#define PAGEADDR            0x22 ///< See datasheet
#define SETCONTRAST         0x81 ///< See datasheet
#define CHARGEPUMP          0x8D ///< See datasheet
#define SEGREMAP            0xA0 ///< See datasheet
#define DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define DISPLAYALLON        0xA5 ///< Not currently used
#define NORMALDISPLAY       0xA6 ///< See datasheet
#define INVERTDISPLAY       0xA7 ///< See datasheet
#define SETMULTIPLEX        0xA8 ///< See datasheet
#define DISPLAYOFF          0xAE ///< See datasheet
#define DISPLAYON           0xAF ///< See datasheet
#define COMSCANINC          0xC0 ///< Not currently used
#define COMSCANDEC          0xC8 ///< See datasheet
#define SETDISPLAYOFFSET    0xD3 ///< See datasheet
#define SETDISPLAYCLOCKDIV  0xD5 ///< See datasheet
#define SETPRECHARGE        0xD9 ///< See datasheet
#define SETCOMPINS          0xDA ///< See datasheet
#define SETVCOMDETECT       0xDB ///< See datasheet

#define SETLOWCOLUMN        0x00 ///< Not currently used
#define SETHIGHCOLUMN       0x10 ///< Not currently used
#define SETSTARTLINE        0x40 ///< See datasheet

#define EXTERNALVCC         0x01 ///< External display voltage source
#define SWITCHCAPVCC        0x02 ///< Gen. display voltage from 3.3V

#define RIGHT_HORIZONTAL_SCROLL              0x26 ///< Init rt scroll
#define LEFT_HORIZONTAL_SCROLL               0x27 ///< Init left scroll
#define VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A ///< Init diag scroll
#define DEACTIVATE_SCROLL                    0x2E ///< Stop scroll
#define ACTIVATE_SCROLL                      0x2F ///< Start scroll
#define SET_VERTICAL_SCROLL_AREA             0xA3 ///< Set scroll range


/*** FUNCTIONS ***/

// convert 0bABCD to 0bAABBCCDD to create 2x2 pixels
byte spread_table[] = {
  0x00, 0x03, 0x0C, 0x0F, 0x30, 0x33, 0x3C, 0x3F,
  0xC0, 0xC3, 0xCC, 0xCF, 0xF0, 0xF3, 0xFC, 0xFF,
};

void command(byte c) {
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(0x00);
  Wire.write(c);
  Wire.endTransmission();
}

void display() {
  command(PAGEADDR);
  command(0x00);
  command(0xFF);
  command(COLUMNADDR);
  command(0x00);
  command(OLED_WIDTH - 1);

  int count = OLED_WIDTH * ((OLED_HEIGHT + 7) / 8);
  byte bytesOut = 1;
  byte * ptr = buf;

  int p_col = 0;
  bool first_pass = true;

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(0x40);
  while (count) {
    if (bytesOut >= WIRE_BUFFER_SIZE) {
      Wire.endTransmission();
      Wire.beginTransmission(I2C_ADDR);
      Wire.write(0x40);
      bytesOut = 1;
    }
    byte p = *ptr++;

    byte k;

    if (first_pass) {
      k = spread_table[p & 0xF]; // low
    } else {
      k = spread_table[p >> 4]; // high
    }
    Wire.write(k); Wire.write(k); // write twice
    bytesOut += 2;
    count -= 2;

    p_col++;

    // redraw line if we're at the end
    if (p_col == P_WIDTH) {
      if (first_pass) { // draw next nibble
        ptr -= P_WIDTH;
        first_pass = false;
      } else {
        first_pass = true;
      }
      p_col = 0;
    }
  }
  Wire.endTransmission();
}

void fillScreen(byte c) {
  memset(buf, c, P_WIDTH * ((P_HEIGHT + 7 / 8)));
}

void clearDisplay() {
  fillScreen(BLACK);
}

void setPixel(byte x, byte y, byte c) {
  if (c == WHITE) {
    buf[x + (y / 8)*P_WIDTH] |= (1 << (y & 7));
  } else if (c == BLACK) {
    buf[x + (y / 8)*P_WIDTH] &=  ~(1 << (y & 7));
  }
}
