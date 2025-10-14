libtcg: lib/libtcg/
	cd lib/libtcg && make all

pico: make_pico.spec crystal-palace/ src/breakpoint_hook.c  src/payload.c
	mkdir -p bin out
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/breakpoint_hook.c -o bin/breakpoint_hook.o
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/payload.c -o bin/payload.o
	java -Dcrystalpalace.verbose=false -classpath crystal-palace/crystalpalace.jar crystalpalace.spec.LinkSpec buildPic ./make_pico.spec x64 out/hwbp_pico.o

runner: make_runner.spec crystal-palace/ src/runner.c
	make libtcg
	make pico
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/runner.c -o bin/runner.o
	java -Dcrystalpalace.verbose=false -classpath crystal-palace/crystalpalace.jar crystalpalace.spec.LinkSpec buildPic ./make_runner.spec x64 out/runner.bin
