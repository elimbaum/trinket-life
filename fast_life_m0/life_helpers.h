#include <stdint.h>
#include <limits.h>
#include <stdlib.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_DotStar.h>

#include <ArduinoLowPower.h>

#include "constants.h"

#define RAND_LONG() ((rand() << 16) ^ rand())

extern uint32_t board[];
extern uint32_t generation;

extern Adafruit_SSD1306 display;

static uint8_t bit_counts[] = {
    0,  // 0
    1,  // 1
    1,  // 2
    2,  // 3
    1,  // 4
    2,  // 5
    2,  // 6
    3   // 7
};

void randomizeBoard();
void drawCell(int, int, int);
// uint32_t rotate_shift_three(uint32_t x, int n);
void collectRandomness(int);
void drawTitle();
void resetGame(uint32_t, uint32_t);

void shutdownAnimation();
void wakeup();

static inline uint32_t rotate_shift_three(uint32_t x, int n) {
    const unsigned int mask = (CHAR_BIT*sizeof(x) - 1);

    // handle wraparound
    if (n == 0) {
        n = 31;
    } else {
        n -= 1;
    }

    n &= mask;
    return ((x >> n) | ((x << ((-n) & mask)))) & 0x7;
}