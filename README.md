# trinket-life
Arduino (trinket m0) implementation of conway's game of life

writeup: https://elibaum.com/2021/02/28/pandemic-projects-tiny-life.html

These files were originally not all in the same place, so I combined into a single git repo. Sorry it's not that well organized. :)

The main and final code is in `fast_life_m0`, if you want to try it out.

## Files
- `fast_life_m0` the main project.
- `hashdata` looking at hashes and game lengths. didn't find much interesting.
- `oled_life` the original code (for Arduino)
- `oled_life_trinket-old` the slighly-updated-code for ATtiny Trinket. I never got this fully working - there probably wasn't enough RAM.
- `oled_test`, `rand_test`, `toggle`: testing out various parts of the system individually.
- `notes.txt` notes from working on this project
- `scanned notes.pdf` some hand-written notes from when I was trying to work out state machines and various other optimizations.

*I didn't have the most up-to-date display library installed!* Hence the weird half-screen thing. My OLED test program was working, because I was basing that off of the new version. Super fkin annoying!

Culprit appears to be that the I2C bus wasn't being allowed to send enough data. Fixed [here](https://github.com/adafruit/Adafruit_SSD1306/compare/2.4.0...2.4.1).