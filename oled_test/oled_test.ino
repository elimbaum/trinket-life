#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define I2C_ADDR 0x3C

// No reset pin on this display.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int frame = 0;
long start_t = 0;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A0));
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.display();

  start_t = millis();
}

void loop() {
  /*
  display.clearDisplay();

  for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
    display.drawPixel(i % SCREEN_WIDTH, i / SCREEN_WIDTH, random(10) < 2);
  }

  display.display();
  */
//  display.fillScreen(1);
  display.fillRect(0, 0, 128, 64, 1);
  display.display();
  frame++;

  delay(1000);
  
  display.fillRect(0, 0, 128, 64, 0);
  display.display();
  frame++;

  if (frame % 100 == 0) {
    long elapsed = millis() - start_t;
    Serial.println(elapsed / 100);
    frame = 0;
    start_t = millis();
  }
  
  delay(1000);
}
