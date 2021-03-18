# A tiny Conway's Game of Life

**TODO**

- make this shorter. general.
- the only interesting thing, really, is the hash, maybe conversion to 32bit.

I love Conway's Game of Life, and I had a small OLED display lying around that I thought would be perfect for a tiny Life-in-a-box contraption.

## Primordial Soup
gif of purple processing version

I wrote a Processing/Java implementation of Life a few years ago that I wanted to adapt for an embedded system. Of course, nested loops and gratuitous function calls are fine on a machine running at a few GHz, but hardly for a 16 MHz (or, as I would later attempt, 8 MHz) microcontroller.

But first, I just ported my code to run on an Arduino wired to the OLED display. This display is really dense - 128x64 but only XXX on the longest side - so running at full resolution didn't look that good. I settled on a display scale factor of 2, giving me a 64x32 effective display size.

Life usually operates on an infinite board, since finite boards create boundary conditions at the edges that are generally undesirable (you tend to get a lot of detritus around the borders). Here, I implemented a toroidal, or donut-shaped, board, which is just to say that the top and bottom are connected, as are the sides.

## The End of the World

Once I had a basic implementation working, I had to figure out how to keep things interesting: on a board as small as the one I am running on, complex structures are hard to come by, and most random initial configurations settle into a somewhat boring equilibrium after about a thousand generations. I wanted to figure out some way to detect the "end" of a round, at which point I could reset the board with a new random configuration.

Note that, since Life is Turing-complete, I believe this question reduces to the Halting Problem, so there's no reason to believe it is easily doable.

My first attempt at equilibrium-state detection was to just count the number of alive cells in each frame. This seemed to work fine, especially at the beginning of a round, but there are a lot of false positives towards the end: for example, if a glider was moving across the board, the number of living cells wouldn't change, and we would incorrectly restart.

Next, I thought about generating a hash of the board. This proved very promising.

XXX djb2a...

XXX list of structures from census

Looking at the Life census, and the equilibrium states of my implementation, I realized that most oscillators had a period of 2. Period-3 oscillators are possible but unlikely, and beyond that, I'm willing to risk that one shows up and hangs the system - I'll probably want to see that happening, if it does. So I settled on keeping the last six hashes, which would catch any end states consisting *only* of period-2 and period-3 oscillators.

(While I initially stored a list of the last six hashes, and checked all of them each time, it turns out that this is not actually necessary: you can just  save off the hash every six generations, and compare against that one.)

One thing this doesn't catch, however: gliders moving unobstructed, in an infinite loop, across the board. This happens less than 1% of the time, but it does happen. I racked my brain for a while trying to figure out a solution, until I realized I can just extend my short-timescale hash with a long-term hash. A glider moves 1 cell every 4 ticks, so to move across all 64 horizontal cells of the board, it will take 256 ticks. To be safe, I used a prime near 256, 263, to ensure that this wouldn't be foiled by, say, some 16-period compound structure that was't caught by the short-term period-6 hash. (This is, admittedly, exceedingly unlikely to happen.)

## Moving to an Adafruit Trinket

My idea for this project was to package the display up with an Adafruit Trinket, a tiny Arduino-based board with an ATtiny85. That chip does _not_ pack a punch. Compared to an Arduino Uno, which I had done the intial development on, the Trinket has:

- spec
- spec
- spec

The Adafruit OLED library I used on the Arduino also was not compatible with the Trinket, so I had to port a subset of that (just enough to turn on the display and draw rectangles) to the Trinket. In particular, since so little memory is available on the ATtiny, I couldn't actually implement a full display buffer: a 128x64 pixel screen requires 1024 bytes for the buffer. Instead, I stored a scaled display buffer at my display resolution of 64x32 (which only requires 256 bytes).

I was actually able to get the display working on the Trinket, and even display some basic test patterns. But with only 256 bytes of RAM left for the entire Arduino core and my code, anything more complicated failed to run.

I made a few other attempts to decrease RAM usage, but none of these ended up panning out. I did eventually get the _stack_ memory usage below the minimum, at least as reported by the Arduino IDE, but then there would surely have been heap allocations to contend with.

- Use direct display buffer access rather than calling `getPixel` and `drawPixel` functions
- Actually just using the display buffer as my board (rather than having a separate buffer for the game state)
- Using bytes instead of integers where possible (but it broke my wraparound code for some reason)

Even if I had gotten it working (say, in raw C without using any of the Arduino core) I imagine it would have been too slow for what I wanted, and I had a much better option available...

## Moving to a much better Adafruit Trinket

Adafruit released an upgraded version of the Trinket, the Trinket M0, which sports a 32-bit ARM processor running at 48 MHz, in the same form-factor as their original board. It's a beast compared to an Arduino, let alone to the XXX Trinket I was struggling to get working before.

There's honestly not much to say here because it worked great right out of the box. With only a few changes, my original code ran at 15 fps (about 70 ms per generation).

I did make a few more changes, however, mostly out of curiousity, interest, and a probably-unhealthy obsession with unnecessary optimizations:

- Since I am now using a 32-bit machine, and my board is 32 cells tall… I can store one column in a single `uint32_t`, rather than a horribly-inefficient array of booleans.

- I used lookup tables to improve the neighbor-counting code, rather than a nested for loop. Simply shift each column by the current y coordinate (minus 1), pull out the three least significant bits, and use that to index the lookup table. This requires 3 additions (left + center + right), rather than 9 (not including the loop overhead).

- To handle top-bottom wrapping, I don't need to any funky modular arithmetic, as I was doing before. Instead, I can use bitwise rotates! There's no C operator that does this, but gcc actually recognizes this code:
    ```
    n &= mask;
    return ((x >> n) | ((x << ((-n) & mask))));
    ```
as *"rotate `x` right by `n` places"*, and compiles it down to a single instruction (`ROR`). (`mask` just enforces the bit-width of the machine.)

- I had settled on a random-generation density of 20% as being a nice value. However, I realized that the Arduino `random` function just calls C's `rand`, and mod's the result by your requested max val. This is kind of a waste: I can just use `rand` directly to generate 32-bit ints, rather than setting each cell individually! This gives me a 50% bit density, however. To get down to 25% (close enough!), I generate two random numbers and bitwise-and them:

    ```
       0 1 1 0 0 1 0 1  50% density
    &  1 0 1 0 1 1 0 0  50% density
    ==================
       0 0 1 0 0 1 0 0  25% density
    ```

Using integers to store the columns was really an extra change. :)

After all this, I had things down to 37 ms per generation - that's too fast to even see. But I couldn't stop! I made a few modifications to the drawing routines, and inlined the bit-counting functions, bringing things down to 33 ms.

Don't get me wrong — this is plenty fast — but I was a bit curious why my seemingly-straightforward code was taking as long as it was. At 48 MHz, 33 ms per generation comes out to about 800 cycles per cell: maybe a few hundred instructions. Sure, there's going to be some overhead outside the main loop body, I told myself, but surely a few hundred instructions to count neighboring pixels and compute the next state of the cell is too much?

But just as I am writing this post, I came across my notes from an earlier test: I had been curious what *the absolute maximum* speed was, assuming any timing limits would probably come from the display update. Lo and behold, toggling a *single pixel* on and off took 28 ms per frame. No wonder I can't get the code to run any faster! In my optimization-haze I seem to have forgotten about these results...

Redoing my math from above, if the display update takes 28 ms, my code is running in 5 ms (for 33 ms total per frame). With 2048 cells on the board, that's about 2 µs per cell. TWO! Which comes out to 96 cycles. I'll take a few dozen instructions per cell! That makes me feel better.

op optimization: https://softwareengineering.stackexchange.com/questions/80084/is-premature-optimization-really-the-root-of-all-evil

(a few days later, I realized the issue was not the display, per se, but just I2C running at the default clock rate of 400 kHz. The display has 8192 pixels, plus overheard, easily gets up to 28 ms. That’ll do it!)


## Hardware and Enclosures

pic of bmo

Originally I thought it would be fun to put this inside of a 3D-printed model of BMO, from Adventure Time. I've never seen the show but I think he's an awesome character. However I don't have easy access to a 3D printer during lockdown, so for now I'm just going to solder this onto a protoboard, and figure out the enclosure later.

I also added a power button so I don't need to unplug the Trinket to turn
everyyhing off. xxx

pic



gif of it running