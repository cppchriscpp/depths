# Frequently Asked Questions

This section tries to address common questions and answers for new developers.

If you have a question that isn't addressed here, feel free to oepn a Github issue, or reach out on
Twitter/somewhere else.

TODO: Link chapter references before, once chapters are a little more solidified.

#### Can I play my game on a real NES?

Absolutely! This engine produces roms that are completely compatible with the NES - they are not limited
to just emulators. It's actually very easy with the right hardware.

There's a whole chapter about this in section 4! 

#### Can I use this for Game Jams/Coding competitions?

As far as I'm concerned, definitely! I plan to use this for a Ludum Dare at some point myself to see
how it fares.

If the contest allows "base code", or "engine code" or something similar, I believe this qualifies.

This does include some basic music and sprites and levels - if you are required to create
all of your content as part of the Jam, you should be sure to remove this content before the end
of the contest.

Here is a list of all non-engine content to be aware of: 

1. Music and sound effects in the famitracker files in the `sound/` directory.
2. The Tiled maps in the `levels/` directory.
3. Nesst graphics in the `graphics/` directory

#### What is all the `CODE_BANK` / `banked_call()` stuff about?

The NES has very limited space for code, and for graphics/etc. It does not know how to use more than 32 
kilobytes of this data at once. This may not make much sense, since many games are 512kb or 
larger! Our game actually uses 128kb of space by default. How is this possible?

The short answer is, we separate the code into smaller 16k sections, and switch out which ones the
NES knows about at once. (This is known as ROM banking) The `CODE_BANK(number)` syntax tells the compiler
which of these 16k sections the code belongs in. The `banked_call` method runs code that is in one of
these sections.

This is explained in greater detail in the 4th section, in a chapter titled "Understanding and adjusting 
the size of your game."

#### What does `Warning: Memory area overflow` mean?

In short, it means you tried to fit too much data/code into the area given. Check out the chapter titled
"Understanding and adjusting the size of your game" in section 4. 
