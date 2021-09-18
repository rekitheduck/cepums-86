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

- Builds on Windows, should be easy to port to Linux as well
- Has enough implemented instructions to run the xi_8088 BIOS. Instructions are implemented as they're hit
- Flags may not be set correctly by instructions
- Instructions themselves should be mostly correct but some mistakes may have slipped in
- There is a macro called `STRICT8086INSTRUCTIONSET` to make the emulator invalidate instructions not present in the original Intel 8086/8088 documentation. Wikipedia lists some instructions (e.g. `0x83/1` OR variant) as "since 80186" in the 8086/8088 category. I don't have physical hardware to test these on, but NASM appears to use these instructions when set to 8086 mode so I'm unsure if these exist in real hardware
- While the clocks should run at their correct frequencies (4.77 MHz for CPU, 1.19 MHz for PIT), instruction timings aren't implemented so instructions execute faster than on a real processor
- Floppy disk controller isn't finished so it can't boot yet
- Port 80h is used by BIOS to output debug information
- Many parts of the system (like the PIC and speaker) are just stubs and don't have any functionality
- Interrupts are supported but are kinda clunky to use
- CMOS settings aren't kept between reboots - they are hardcoded

## Why?

Because I've wanted to build something like this for a while. And, this should give me enough information to help design my own microcomputer