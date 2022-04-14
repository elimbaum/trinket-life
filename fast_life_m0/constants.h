#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define BOARD_WIDTH 64
#define BOARD_HEIGHT 32

// #define WHITE 1
// #define BLACK 0

#define SCALE 2

#define I2C_ADDR 0x3C

// how often to take a hash
#define SHORT_HASH_INTERVAL 6
#define LONG_HASH_INTERVAL 256

// fraction of 32767 to randomly reset if long hash matches
#define LONG_HASH_THRESHOLD 328

#define FRAME_DELAY 64
#define FRAME_TIME 20

#define BUTTON_PIN 3
#define DEBOUNCE_MS 100

#define SDA_PIN 0
#define SCK_PIN 2

#define SERIAL_TIMEOUT 5000