# Hardware Breakpoint PICO

Still kind of a work in progress. Run `make build` to build, but you'll need to provide your own `crystal-palace` directory with the CPL executables. Resulting shellcode in `out.bin` will hook `VirtualFree()` and pop up a dialog box when the breakpoint is hit. No support for breakpoint unhooking or cleanup yet.

Thanks to Raphael Mudge for [Crystal Palace](https://tradecraftgarden.org/crystalpalace.html) and the PICO runner code, Rastamouse for a [blog post](https://rastamouse.me/modular-pic-c2-agents-reprise/) that helped me understand how to merge COFF files with Crystal Palace, and Rad98 for [this implementation of hardware breakpoint hooking](https://github.com/rad9800/hwbp4mw) that I used as a template for this PICO.
