# Hardware Breakpoint PICO

XXX

The source code is divided up into 3 files:

- **breakpoint_hook.c**: Contains the PICO's entrypoint and breakpoint hooking logic. Merged with `payload.c` to create the PICO.
- **payload.c**: Contains the function that is exected when the breakpoint is triggered. Merged with `breakpoint_hook.c` to create the PICO.
- **runner.c**: A simple PICO runner that puts a breakpoint on `VirtualFree()` just before it gets called. Intended to demonstrated the functionality.

Run `make build` to compile and link the PICO and runner. It will work out of the box as long as you have a `crystal-palace` directory with all of the CPL executables. The resulting shellcode will be written to `out.bin`, and will hook `VirtualFree()` to pop up a dialog box.

This is still kind of a work in progress. TODO:

- Add support for breakpoint unhooking and cleanup.
- Write a more useful runner that doesn't rely on a hardcoded pointer, and allows you to specify what should be hooked instead.

Thanks to Raphael Mudge for [Crystal Palace](https://tradecraftgarden.org/crystalpalace.html) and the PICO runner code, Rastamouse for a [blog post](https://rastamouse.me/modular-pic-c2-agents-reprise/) that helped me understand how to merge COFF files with Crystal Palace, and Rad98 for [this implementation of hardware breakpoint hooking](https://github.com/rad9800/hwbp4mw) that I used as a template for this PICO.
