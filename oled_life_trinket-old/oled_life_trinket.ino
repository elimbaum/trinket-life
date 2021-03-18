/* OLED Life on Trinket (ATtiny85)

   Original (Uno) usage:
    15450 flash / 696 RAM (not inclu locals)
"
   Intial re-compile:
    2564 flash / 342 RAM
*/

#include <Wire.h>
#include "patterns.h"
#include "ssd1306.h"

#define WIDTH  64
#define HEIGHT 32

#define RANDOM_PERCENT 20

#define NUM_SEED_SAMPLES 16

byte * buffer;
#define GET_PIXEL(x, y) (buffer[(x) + ((y) / 8) * WIDTH] & (1 << ((y) & 7)))

// 3 bit hamming weights
// (i.e. how many 1s)
// use this instead of adding up live pixels
byte hamming_weight[] = { 0, 1, 1, 2, 1, 2, 2, 3 };f

// game is 32 pixels high so just use a long
unsigned long first_column;
unsigned long prev_column;
unsigned long this_column;
unsigned long next_column;

// want to be able to take care of 2- and 3-period oscillators
// this could theoretically pick up a higher-order oscillator, but hopefully
// not too often.
#define HASH_LENGTH 6
unsigned long hashList[HASH_LENGTH];
byte hashIndex = 0;

void randomizeDisplay() {
  clearDisplay();
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    if (random(100) < RANDOM_PERCENT) {
      setPixel(i % WIDTH, i / WIDTH, WHITE);
    }
  }
}

// custom draw bitmap to account for scale factor
void drawPattern(const uint8_t bitmap[], int w, int h) {
  clearDisplay();

  int byteWidth = (w + 7) / 8;
  byte b = 0;

  int start_x = (WIDTH - w) / 2;
  int start_y = (HEIGHT - h) / 2;

  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (i & 7) {
        b <<= 1;
      } else {
        b = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
      }

      if (b & 0x80) {
        setPixel(i + start_x, j + start_y, WHITE);
      }
    }
  }
}

void setup() {
  unsigned long seed = 0;
  for (int i = 0; i < NUM_SEED_SAMPLES; i++) {
    // TODO probably not A0, change
    seed += analogRead(A0) << i;
    delay(10);
  }

  randomSeed(seed);

  randomizeDisplay();
  //drawPattern(r_pentomino, 3, 3);
  display();
  delay(1000);
}

void loop() {
  // compute the djb2a hash
  unsigned long hash = 5381;

  /*** BEGIN BOARD LOOP ***/
  for (int x = 0; x < WIDTH; x++) {
    byte c = 0;
    // load next column
    for (int y = 0; y < HEIGHT; y++) {
      c <<= 1;

      // default
      this_column &= ~(1 << y);

      // pull out the 3-bit sequence (shift and bitwise and)
      byte n = hamming_weight[prev_column >> (y % 8) & 0x7]
               + hamming_weight[this_column >> (y % 8) & 0x7]
               + hamming_weight[next_column >> (y % 8) & 0x7];

      // check neighbors
      //      for (int xs = x - 1; xs <= x + 1; xs++) {
      //        for (int ys = y - 1; ys <= y + 1; ys++) {
      //          if (GET_PIXEL(((xs + WIDTH) % WIDTH) * SCALE,
      //                        ((ys + HEIGHT) % HEIGHT) * SCALE)) {
      //            n++;
      //          }
      //        }
      //      }

      boolean thisPixel = GET_PIXEL(x, y);

      // n = thisPixel ? ((n - 1) | 1) : n;
      // if (n == 3) {
      //   this_column[y] = true;
      //   c++;serial
      // }

      if (thisPixel) {
        // counted above, ignore
        n--;
      }


      if ((n == 3) || (n == 2 && thisPixel)) {
        this_column |= 1 << y;
        c++;
      }

      // byte batch hash (@ y = 7, 15, 23, ...)
      if (y & 7 == 7) {
        hash = ((hash << 5) + hash) ^ c;
        c = 0;
      }
    }
    // END y loop

    // final batch
    hash = ((hash << 5) + hash) ^ c;

    // draw the previous column... don't need it anymore!
    // but skip first column, so wrap around still works
    if (x > 1) {
      for (byte y = 0; y < HEIGHT; y++) {
        setPixel(x - 1, y, (prev_column >> y) == 0);
      }
    }

    memcpy(prev_column, this_column, sizeof(this_column));
    memcpy(this_column, next_column, sizeof(next_column));

    // save first column for later
    if (x == 0) {
      memcpy(first_column, this_column, sizeof(this_column));
    }
  }

  /*** END BOARD LOOP ***/

  // draw the first and last columns
  for (byte y = 0; y < HEIGHT; y++) {
    setPixel(0, y, first_column >> y == 0);
    setPixel(WIDTH - 1, y, prev_column >> y == 0);
  }

  // hash checks
  boolean reset = false;
  for (byte i = 0; i < HASH_LENGTH; i++) {
    if (hashList[i] == hash) {
      // restart!
      reset = true;

      delay(2000);
      fillScreen(WHITE);
      display();
      delay(1000);

      randomizeDisplay();
    }
  }

  if (! reset) {
    // save hash
    hashList[hashIndex] = hash;
    hashIndex++;

    if (hashIndex >= HASH_LENGTH) {
      hashIndex = 0;
    }

  }

  display();

}
