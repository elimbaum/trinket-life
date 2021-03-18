#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_DotStar.h>

#include "patterns.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SCALE 2
#define WIDTH  (SCREEN_WIDTH / SCALE)
#define HEIGHT (SCREEN_HEIGHT / SCALE)

#define RANDOM_PERCENT 20

#define WHITE 1
#define BLACK 0

#define I2C_ADDR 0x3C

// extra delay between frames, ms
// this isn't actually the frame rate, because the computation
// for each frame takes a while. (~66 ms on M0)
#define FRAME_DELAY 0

// No reset pin on this display.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_DotStar LED = Adafruit_DotStar(1, INTERNAL_DS_DATA,
                                        INTERNAL_DS_CLK,
                                        DOTSTAR_BGR);

byte first_column[HEIGHT];
byte prev_column[HEIGHT];
byte this_column[HEIGHT];

byte * buffer;
#define GET_PIXEL(x, y) (buffer[(x) + ((y) / 8) * SCREEN_WIDTH] & (1 << ((y) & 7)))


// want to be able to take care of 2- and 3-period oscillators
// this could theoretically pick up a higher-order oscillator, but hopefully
// not too often.
// period 3 oscillators are themselves already pretty rare.
#define HASH_LENGTH 6
unsigned long hashList[HASH_LENGTH];
int hashIndex = 0;

// also want to catch gliders that go on forever. To do this, keep a hash
// that updates with a large (prime) period.
// by using a prime, the hash we are saving should almost never be the same,
// since no oscillators will repeat in that amount of time.
//
// quick math: screen size 64. gliders move 1 cell per 4 ticks, so a glider
// should return to its original position on this board after 256 ticks.
// we want a prime near there, may a bit more.
#define HASH_PRIME_INTERVAL 263
unsigned long prime_hash = 0;

// for seed
#define NUM_RAND_SAMPLES 100

unsigned long t_start;
unsigned int generation = 0;

// equivalent: display.fillRect(x * SCALE, y * SCALE, SCALE, SCALE, c)
// this may not actually be faster (70 ms vs. 66 ms per gen)
void drawScaledPixel(int x, int y, int c) {
  display.drawPixel(x * SCALE, y * SCALE, c);
  display.drawPixel(x * SCALE + 1, y * SCALE, c);
  display.drawPixel(x * SCALE, y * SCALE + 1, c);
  display.drawPixel(x * SCALE + 1, y * SCALE + 1, c);
}

void randomizeDisplay() {
  display.clearDisplay();
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    if (random(100) < RANDOM_PERCENT) {
      display.fillRect(i % WIDTH * SCALE, i / WIDTH * SCALE, SCALE, SCALE, WHITE);
      // drawScaledPixel(i % WIDTH, i / WIDTH, WHITE);
    }
  }
}

// custom draw bitmap to account for scale factor
void drawPattern(const uint8_t bitmap[], int w, int h) {
  display.clearDisplay();

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
        display.fillRect((i + start_x) * SCALE, (j + start_y) * SCALE,
                         SCALE, SCALE, WHITE);
      }
    }
  }
}

void title() {
  int x = 8;
  int y = 4;

  int line_height = 20;
  int x_offset = 8;

  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);

  display.setCursor(x, y);
  display.print("Game");
  display.display();

  delay(1000);
  x += x_offset;
  y += line_height;

  display.setCursor(x, y);
  display.print("of");
  display.display();

  delay(1000);
  x += x_offset;
  y += line_height;

  display.setCursor(x, y);
  display.print("Life");
  display.display();

  delay(3000);

  display.fillScreen(BLACK);
  display.display();
}

void resetGame(unsigned long hash) {
  // restart!
  unsigned long gen_time = millis() - t_start;
  Serial.print("MATCH "); Serial.print(hash, HEX);
  Serial.print(" after generation "); Serial.print(generation);
  Serial.print(" ("); Serial.print(gen_time); Serial.print(" ms, ");
  Serial.print(gen_time / generation); Serial.println(" ms/gen)");

  delay(1000);

  display.fillScreen(BLACK);

  display.setTextSize(2);
  display.setCursor(4, 24);
  display.setTextColor(WHITE, BLACK);
  display.print("Age: ");
  display.print(generation);

  generation = 0;

  display.display();
  delay(3000);

  display.fillScreen(BLACK);
  display.display();
  delay(1000);

  randomizeDisplay();

  t_start = millis();
}

void setup() {
  Serial.begin(9600);
  LED.begin(); LED.clear(); LED.show();

  if (!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.fillScreen(BLACK);

  buffer = display.getBuffer();

  //  title();

  Serial.println();

  // collect one second of "randomness"
  long wait_t = millis();
  int r = 0;
  while (wait_t + 1000 > millis()) {
    r += analogRead(A3);
    delay(1);
  }
  randomSeed(r);
  Serial.print("Random seed: ");
  Serial.println(r);

  //  drawPattern(pentadecathlon, 10, 3);
  randomizeDisplay();
  display.display();
  delay(500);

  t_start = millis();
}

void loop() {
  // compute the djb2a hash
  unsigned long hash = 5381;

  /*** BEGIN BOARD LOOP ***/
  for (int x = 0; x < WIDTH; x++) {
    byte c = 0;
    for (int y = 0; y < HEIGHT; y++) {
      c <<= 1;

      // default
      this_column[y] = false;
      byte n = 0;

      // check neighbors
      for (int xs = x - 1; xs <= x + 1; xs++) {
        for (int ys = y - 1; ys <= y + 1; ys++) {
          if (GET_PIXEL(((xs + WIDTH) % WIDTH) * SCALE,
                        ((ys + HEIGHT) % HEIGHT) * SCALE)) {
            n++;
          }
        }
      }

      bool thisPixel = GET_PIXEL(x * SCALE, y * SCALE);

      // n = thisPixel ? ((n - 1) | 1) : n;
      // if (n == 3) {
      //   this_column[y] = true;
      //   c++;
      // }

      if (thisPixel) {
        // counted above, ignore
        n--;
      }


      if ((n == 3) || (n == 2 && thisPixel)) {
        this_column[y] = true;
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
        display.fillRect((x - 1) * SCALE, y * SCALE, SCALE, SCALE, prev_column[y]);
        // drawScaledPixel(x - 1, y, prev_column[y]);
      }
    }

    memcpy(prev_column, this_column, sizeof(this_column));

    // save first column for later
    if (x == 0) {
      memcpy(first_column, this_column, sizeof(this_column));
    }
  }

  /*** END BOARD LOOP ***/

  // draw the first and last columns
  for (byte y = 0; y < HEIGHT; y++) {
    display.fillRect(0, y * SCALE, SCALE, SCALE, first_column[y]);
    display.fillRect((WIDTH - 1) * SCALE, y * SCALE, SCALE, SCALE, prev_column[y]);
    // drawScaledPixel(0, y, first_column[y]);
    // drawScaledPixel(WIDTH - 1, y, prev_column[y]);
  }

  // hash checks
  boolean reset = false;

  // check the recent list (p2 or p3 oscillators)
  for (byte i = 0; i < HASH_LENGTH; i++) {
    if (hashList[i] == hash) {
      resetGame(hash);
      reset = true;
      break;
    }
  }

  // check prime hash
  if (hash == prime_hash && !reset) {
    Serial.println("Prime hash match!");
    resetGame(hash);
    reset = true;
  }

  if (! reset) {
    // save new hash
    hashList[hashIndex] = hash;
    hashIndex++;

    if (hashIndex >= HASH_LENGTH) {
      hashIndex = 0;
    }

    if (generation % HASH_PRIME_INTERVAL == 0) {
      prime_hash = hash;
    }

    generation++;
  }

  display.display();
  delay(FRAME_DELAY);

}
