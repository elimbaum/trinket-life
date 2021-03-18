#include <Adafruit_DotStar.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BUTTON 3
#define BOUNCE_TIME 100

Adafruit_SSD1306 display(128, 64, &Wire, -1);

Adafruit_DotStar LED = Adafruit_DotStar(1, INTERNAL_DS_DATA,
                                        INTERNAL_DS_CLK,
                                        DOTSTAR_BGR);

#define S(name) #name

typedef enum button_state_t {
    NOT_PRESSED,
    PRESSING,
    PRESSED,
    RELEASING
} ButtonState;

volatile ButtonState state = NOT_PRESSED;

const char * button_state_names[] = {
    S(NOT_PRESSED), S(PRESSING),
    S(PRESSED), S(RELEASING)
};

uint32_t colors[] = {
    LED.Color(255, 127, 0),
    LED.Color(255, 0, 255),
    LED.Color(255, 255, 0),
    LED.Color(0, 255, 255),
    LED.Color(127, 127, 127),
    LED.Color(0, 127, 255),
};

const int color_len = sizeof(colors)/sizeof(uint32_t);

volatile int i = 0;
volatile uint32_t last_button_press_t = 0;
volatile uint32_t last_button_release_t = 0;
volatile bool pressed = false;
volatile bool timer_expired = false;
volatile int pr = 0;
volatile int re = 0;
volatile bool print = false;
volatile bool last_state = false;

void button_interrupt() {
    uint32_t now = millis();

    print = true;
    if (digitalRead(BUTTON)) {
        // button RELEASE
        if (pressed && now - last_button_release_t >= BOUNCE_TIME) {
            re++;
            pressed = false;
            i++;
        }
        last_button_release_t = now;
    } else {
        // button PRESS
        if (! pressed && now - last_button_press_t >= BOUNCE_TIME) {
            pr++;
            pressed = true;
            
        }
        last_button_press_t = now;
    }
}



void setup() {
    pinMode(BUTTON, INPUT_PULLUP);

    Serial.begin(9600);

    LED.begin();
    LED.setBrightness(10);
    LED.show();

    // while (! Serial) {}

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }

    attachInterrupt(digitalPinToInterrupt(BUTTON), button_interrupt, CHANGE);
}

int last_i = 0;
char line[32];

void loop() {
    // did timer expire?
    // int now = millis();
    // if (! timer_expired && now - last_button_event_t > BOUNCE_TIME) {
    //     // yup!
    //     Serial.println("timer expired");
    //     timer_expired = true;

    //     bool pressed = ! pressed;
    // }

    // LED.setPixelColor(0, LED.gamma32(colors[i]));
    // LED.show();

    // if (print) {
    //     print = false;
    //     sprintf(line, "Pr: %d\tRe:%d\ti: %d\t%s",
    //             pr, re, i, pressed ? "PRESSED" : "NOT_PRESSED");
    //     Serial.println(line);
    // }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE, BLACK);

    display.setCursor(4, 8);
    display.print(i);

    display.setCursor(4, 32);
    display.print(i % 2 ? "AWAKE" : "SLEEP");
    
    display.display();
}