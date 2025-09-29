build: make_pico.spec make_runner.spec crystal-palace/ src/breakpoint_hook.c  src/payload.c  src/runner.c src/headers/
	mkdir -p bin out
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/runner.c -o bin/runner.o
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/breakpoint_hook.c -o bin/breakpoint_hook.o
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/payload.c -o bin/payload.o
	java -Dcrystalpalace.verbose=false -classpath crystal-palace/crystalpalace.jar crystalpalace.spec.LinkSpec buildPic ./make_pico.spec x64 out/hwbp_pico.o
	java -Dcrystalpalace.verbose=false -classpath crystal-palace/crystalpalace.jar crystalpalace.spec.LinkSpec buildPic ./make_runner.spec x64 out/runner.bin
