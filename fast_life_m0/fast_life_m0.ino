#pragma message ("\nBuild " __DATE__ " " __TIME__)

#include "life_helpers.h"
#include "constants.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_DotStar LED = Adafruit_DotStar(1, INTERNAL_DS_DATA,
                                        INTERNAL_DS_CLK,
                                        DOTSTAR_BGR);

uint32_t board[BOARD_WIDTH];

uint32_t first_col;
uint32_t this_col;
uint32_t last_col;

uint32_t check_hash = 0;
uint32_t generation = 0;
uint32_t start_t = 0;

uint32_t frame_start_t = 0;

volatile bool shutdown_request = false;
volatile bool pressed = false;
volatile uint32_t last_button_press_t = 0;
volatile uint32_t last_button_release_t = 0;

void button_interrupt() {
    uint32_t now = millis();

    if (digitalRead(BUTTON_PIN)) {
        // button HIGH -> released
        if (pressed && now - last_button_release_t >= DEBOUNCE_MS) {
            pressed = false;

            // ACTION ON KEY UP
            shutdown_request = true;
        }
        last_button_release_t = now;
    } else {
        // button LOW -> pressed
        if (!pressed && now - last_button_release_t >= DEBOUNCE_MS) {
            pressed = true;

            // ACTION ON KEY DOWN
            // none
        }
        last_button_press_t = now;
    }
}

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(9600);

    // turn off LED
    LED.begin(); LED.clear(); LED.show();

    Serial.println("Game of Life");
    Serial.println("Build " __DATE__ " " __TIME__);

    if (!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDR)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }

    display.fillScreen(BLACK);
    display.display();

    drawTitle();


    Serial.println("Randomizing...");
    // one second of randomness
    collectRandomness(1000);

    display.fillScreen(BLACK);
    
    randomizeBoard();

    //// 1200 pentomino
    // board[31] = 4 << 15;
    // board[32] = 7 << 15;
    // board[33] = 2 << 15;

    display.display();
    delay(1000);

    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        button_interrupt,
        CHANGE
    );
    
    Serial.println("Start!");

    check_hash = 0;
    generation = 0;

    start_t = micros();
    frame_start_t = millis();
}

void loop() {
    if (shutdown_request) {
        // detach sleep interrupt
        detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
        shutdown_request = false;

        Serial.println("shutting down");

        delay(1000);

        shutdownAnimation();

        // turn off display
        display.ssd1306_command(SSD1306_DISPLAYOFF);

        // attach wake interrupt.
        //
        // RISING interrupt will, ideally, catch button as its being released,
        // but may happen during press due to bounce.
        //
        // setup takes long enough that we shouldn't have to worry about
        // debouncing; sleep interrupt reattached at end.
        
        LowPower.attachInterruptWakeup(digitalPinToInterrupt(BUTTON_PIN), wakeup, RISING);
        LowPower.sleep();
        setup();

        // execution continues from here, AFTER setup is run.

        return;
    }

    display.fillScreen(BLACK);

    // Serial.println("start loop");

    uint32_t hash = 5381;

    for (int x = 0; x < BOARD_WIDTH; x++) {
        // Serial.print("x");
        // Serial.println(x);

        uint32_t left;
        if (x == 0) {
            left = board[BOARD_WIDTH - 1];
        } else {
            left = board[x - 1];
        }

        uint32_t center = board[x];

        uint32_t right;
        if (x == BOARD_WIDTH - 1) {
            right = board[0];
        } else {
            right = board[x + 1];
        }

        this_col = 0;

        uint32_t hash_acc = 0;

        for (int y = 0; y < BOARD_HEIGHT; y++) {
            // Serial.print("y");
            // Serial.println(y);

            hash_acc <<= 1;

            // get the bit counts
            uint32_t shift_center = rotate_shift_three(center, y);
            int count = bit_counts[rotate_shift_three(left, y)] +
                        bit_counts[shift_center] +
                        bit_counts[rotate_shift_three(right, y)];
            bool alive = shift_center & 0x2;

            // rule: born if 3, stay alive if 2 or 3.
            // if already alive, however, total "neighbor" count will be +1
            // so check for 3, or (3 and 4 if alive)
            if ( (count == 3) || (count == 4 && alive) ) {
                this_col |= 1 << y;
                hash_acc |= 1;
            }

            // byte-batch hash
            if (y & 0x7 == 0x7) {
                hash = ((hash << 5) + hash) ^ hash_acc;
                hash_acc = 0;
            }
        }
        // end y loop


        // draw previous column
        if (x > 1) {
            board[x - 1] = last_col;
            for (int y = 0; y < BOARD_HEIGHT; y++) {
                if (last_col & 1) {
                    drawCell(x - 1, y, 1);    
                }
                last_col >>= 1;
            }
        } else if (x == 0) {
            // save first col for wrap
            first_col = this_col;
        }

        // save this col
        last_col = this_col;
    }
    // end board loop

    // draw first and last cols
    board[0] = first_col;
    board[BOARD_WIDTH - 1] = last_col;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        if (first_col & 1) {
            drawCell(0, y, 1);    
        }
        first_col >>= 1;

        if (last_col & 1) {
            drawCell(BOARD_WIDTH - 1, y, 1);
        }
        last_col >>= 1;
    }

    // check hash
    if (check_hash == hash) {
        resetGame(hash, micros() - start_t);
        generation = 0;
        start_t = micros();
    } else {
        if (generation % HASH_INTERVAL == 0) {
            check_hash = hash;
        }

        // Serial.print(generation);
        // Serial.print(" ");
        // Serial.println(hash);

        generation++;
    }

    // Serial.println("displaying");

    display.display();

    while(millis() - frame_start_t < FRAME_TIME) {
        delay(1);
    }
    frame_start_t = millis();
}