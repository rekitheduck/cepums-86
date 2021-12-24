# Cepums-86 Emulator project

Cepums-86 is an x86 emulator built to help design a homebrew clone of the IBM PC XT microcomputer

## Emulated hardware

- Intel 8086 microprocessor
- Intel 8042 PS/2 keyboard controller (no mouse)
- Intel 8254 programmable interval timer
- MDA graphics adapter
- Real-time clock and CMOS
- Floppy disk controller with support for 4 drives (heavy WIP)

## Current state

- Builds on Windows and Linux
- Has enough implemented instructions to run the 8088_bios BIOS ([modified fork](https://github.com/ciba43/8088_bios)) Instructions are implemented as they're hit
- Flags may not be set correctly by all instructions
- Instructions themselves should be mostly correct but some mistakes may have slipped in
- There is a macro called `STRICT8086INSTRUCTIONSET` to make the emulator invalidate instructions not present in the original Intel 8086/8088 documentation. Wikipedia lists some instructions (e.g. `0x83/1` OR variant) as "since 80186" in the 8086/8088 category. I don't have physical hardware to test these on, but NASM appears to use these instructions when set to 8086 mode so I'm unsure if these exist in real hardware
- While the clocks should run at their correct frequencies (4.77 MHz for CPU, 1.19 MHz for PIT), instruction timings aren't implemented so instructions execute faster than on a real processor
- Floppy disk controller isn't finished so it can't boot yet
- Port 80h is used by BIOS to output debug information
- Many parts of the system (like the PIC and speaker) are just stubs and don't have any functionality
- Interrupts are supported but are kinda clunky to use
- Keyboard activity is relayed to the emulator but certain keys might cause crashes
- CMOS settings aren't kept between reboots - they are hardcoded and should pass the checksum check

## Why?

Because I've wanted to build something like this for a while. And, this should give me enough information to help design my own microcomputer

## How do I build this?

Installation should be relatively easy. First of all, you need the BIOS which is located [here](https://github.com/ciba43/8088_bios). There's a Makefile there that assembles it.
If you're on a platform without `make`, you can use `nasm -DMACHINE_DUCK -O9 -f bin -o bios.bin -l bios.lst bios.asm` to build it.

Then you have to build the emulator itself. The build system can be generated on Windows by running the included batch script `Generate-Win.bat`. The batch script should be modified with the Visual Studio version that you have
On Linux, you'll need to install premake5 and run `premake5 gmake2` to get a Makefile.
Then you can build it either with Visual Studio or by running `make` (depending on your platform).
Then you'll need to copy over the `bios.bin` file and get yourself an (IBM VGA font)[https://github.com/viler-int10h/vga-text-mode-fonts/raw/master/FONTS/PC-IBM/VGA8.F16] and rename it to `default-font.bin`.

Additionally on Windows, you'll need to download SDL2 and put SDL2.dll in the same directory.