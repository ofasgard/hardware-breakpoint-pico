# Hardware Breakpoint PICO

A proof-of-concept hardware breakpoint hooker, implemented as a PICO for Crystal Palace. With this PICO, you can place a hardware breakpoint on an arbitrary address (such as the entrypoint of a WIN32 API) that will invoke your desired payload the next time that address is executed.

The source code is divided up into 3 files:

- **breakpoint_hook.c**: Contains the PICO's entrypoint and breakpoint hooking logic. 
- **payload.c**: Contains the code that is executed when the breakpoint actually triggers.
- **runner.c**: A simple PICO runner that puts a breakpoint on `VirtualFree()` just before it gets called. Intended to demonstrated the functionality.

Run `make build` to compile and link the PICO and runner. It will work out of the box as long as you have MinGW GCC and a `crystal-palace` directory with all of the CPL executables. The resulting shellcode will be written to `out/runner.bin`, and hooks `VirtualFree()` to pop up a dialog box when executed.

![A screenshot of the PICO in action, triggering a dialog box](img/hwbp_pico.png)

This is still kind of a work in progress. TODO:

- Write a more useful runner that doesn't rely on a hardcoded pointer, and allows you to specify what should be hooked instead.

Thanks to Raphael Mudge for [Crystal Palace](https://tradecraftgarden.org/crystalpalace.html) and the PICO runner code, Rastamouse for a [blog post](https://rastamouse.me/modular-pic-c2-agents-reprise/) that helped me understand how to merge COFF files with Crystal Palace, and Rad98 for [this implementation of hardware breakpoint hooking](https://github.com/rad9800/hwbp4mw) that I used as a template for this PICO.
