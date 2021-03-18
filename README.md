# trinket-life
Arduino (trinket m0) implementation of conway's game of life

writeup: https://elimbam.com/2021/02/28/pandemic-projects-tiny-life.html

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