# SteelSeriesControl
Controller for SteelSeries peripherals (OK, just the Rival 700 so far, and only a bit)

This project started because I found that [Flozz's RivalCfg](https://github.com/flozz/rivalcfg) didn't support the Rival 700 mouse, and investigation into the same had halted.

Another interesting project is [libratbag](https://github.com/libratbag/libratbag) but in its current form, I don't know if it would be able to support the Rival 700 OLED, since messages need wValue to be set to 0x0300. This may also affect SteelSeries devices in the future, since most other messages have wValue set to 0x0200, and future mice (or even firmwares for current mice) may elect to reject non-conformant messages.

When I created this Repo, I hadn't touched C++ in about 15 years, so please excuse the quality.

Currently it contains some CMake information. Happy to see this changed if there's a better way.

### Run:

#### SteelSeriesControl Tactile _n_
&nbsp;where<br>
&nbsp;&nbsp;_n_ is a 7-bit integer (0 thru 127) or one of the following: Strong, Soft, Sharp, Ping, Bump, Double, QuickDouble, QuickDoubleSoft, QuickTriple, Buzz, LongBuzz, Ring, LongButLight, LightBuzz, Tick, Pulse, StrongPulse<br>
Synonyms for "Tactile" are "Haptic" and "Buzz"

#### SteelSeriesControl LED _led_ _mode_ _colour1_ _colour2_ _duration_
&nbsp;where<br>
&nbsp;&nbsp;_led_ is Logo or Wheel,<br>
&nbsp;&nbsp;_mode_ is Trigger or Steady (Shift is not yet supported),<br>
&nbsp;&nbsp;_colour1_ is the Steady colour or the at-rest colour for Trigger mode,<br>
&nbsp;&nbsp;_colour2_ is the "active" colour for Trigger mode,<br>
&nbsp;&nbsp;_duration_ is the duration of the fade for Trigger mode, in milliseconds<br>
Synonyms for LED are Light, Lamp, Colour, Color

#### SteelSeriesControl Image _filename_
&nbsp;where<br>
&nbsp;&nbsp;_filename_ points to a 576-byte file representing the bits to display on the screen in left-to-right order starting at 0, and where the MSB is left-most and LSB is right-most.<br>
Synonyms for Image are Picture and OLED

#### SteelSeriesControl Animation
&nbsp;In this mode, SteelSeriesControl will search the current directory for files with the extension *.bits*, sort them, and stream them to the mouse.
