build: crystal-palace/ loader.spec src/breakpoint_hook.c  src/payload.c  src/runner.c src/headers/
	mkdir -p bin
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/runner.c -o bin/runner.o
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/breakpoint_hook.c -o bin/breakpoint_hook.o
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/payload.c -o bin/payload.o
	java -Dcrystalpalace.verbose=false -classpath crystal-palace/crystalpalace.jar crystalpalace.spec.LinkSpec buildPic ./loader.spec x64 ./out.bin
