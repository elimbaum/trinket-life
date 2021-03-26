#include "life_helpers.h"

void randomizeBoard() {
    for (int x = 0; x < BOARD_WIDTH; x++) {
        // Bitwise and two rands to get ~ 25% density.
        uint32_t col;
        col = board[x] = RAND_LONG() & RAND_LONG();

        for (int y = 0; y < BOARD_WIDTH; y++) {
            drawCell(x, y, col & 1);
            col >>= 1;
        }
    }
}


void drawCell(int x, int y, int c) {
    // display.fillRect(x * SCALE, y * SCALE, SCALE, SCALE, c);
    display.drawPixel(x * SCALE, y * SCALE, c);
}

// collect randomness for seed for t millis
void collectRandomness(int t) {
    uint32_t wait_t = millis();
    uint32_t r = 0;

    while (wait_t + t > millis()) {
        r += analogRead(A4);
        delay(1);
    }

    randomSeed(r);
}

void drawTitle() {
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


void resetGame(uint32_t hash, uint32_t gen_time) {
  // restart!
  Serial.print("MATCH "); Serial.print(hash, HEX);
  Serial.print(" after generation "); Serial.print(generation);
  Serial.print(" ("); Serial.print(gen_time / 1000000); Serial.print(" sec, ");
  Serial.print(gen_time / generation); Serial.println(" us/gen)");

  delay(1000);

  display.fillScreen(BLACK);

  display.setTextSize(2);
  display.setCursor(4, 24);
  display.setTextColor(WHITE, BLACK);
  display.print("Age: ");
  display.print(generation);

  display.display();
  delay(3000);

  display.fillScreen(BLACK);
  display.display();
  delay(1000);

  randomizeBoard();
}

void shutdownAnimation() {
  // animate shutdown from edges towards center
  for (int y = 0; y <= SCREEN_HEIGHT / 2; y++) {
    display.fillRect(0, y, SCREEN_WIDTH, 1, BLACK);
    display.fillRect(0, SCREEN_HEIGHT - y, SCREEN_WIDTH, 1, BLACK);
    display.display();

    // delay(10);
  }

  display.clearDisplay();
  display.display();
}

// wakeup ISR
void wakeup() {
    // remove wakeup interrupt
    detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
}