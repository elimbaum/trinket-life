void setup() {
    Serial.begin(115200);
    // needed for native USB
    while (! Serial) {}

    randomSeed(analogRead(A0));
}

uint32_t minv = -1;
uint32_t maxv = 0;

uint32_t even = 0;
uint32_t count = 0;

void loop() {
    uint32_t v = (rand() << 16) ^ rand();

    bool print = false;

    if (v < minv) {
        minv = v;
        print = true;
    }

    if (v > maxv) {
        maxv = v;
        print = true;
    }

    if ((v & 1) == 0) {
        even++;
    }
    count++;

    if (print) {
        Serial.print(count); Serial.print(": ");
        Serial.print(v, HEX); Serial.print("; min ");
        Serial.print(minv, HEX); Serial.print(",  max ");
        Serial.print(maxv, HEX); Serial.print("; even: ");
        Serial.print(100 * even / count); Serial.println("%");
    }
}