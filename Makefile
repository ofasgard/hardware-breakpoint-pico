dirs:
	mkdir -p bin out

libtcg: lib/libtcg/
	cd lib/libtcg && make all
	
libcpltest: lib/LibCPLTest/
	cd lib/LibCPLTest && make all

pico: dirs make_pico.spec crystal-palace/ src/breakpoint_hook.c  src/payload.c
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/breakpoint_hook.c -o bin/breakpoint_hook.o
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/payload.c -o bin/payload.o
	java -Dcrystalpalace.verbose=false -classpath crystal-palace/crystalpalace.jar crystalpalace.spec.LinkSpec buildPic ./make_pico.spec x64 out/hwbp_pico.o

runner: dirs libtcg pico make_runner.spec crystal-palace/ src/runner.c
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/runner.c -o bin/runner.o
	java -Dcrystalpalace.verbose=false -classpath crystal-palace/crystalpalace.jar crystalpalace.spec.LinkSpec buildPic ./make_runner.spec x64 out/runner.bin
	
tester: dirs libtcg libcpltest runner crystal-palace/ make_tester.spec src/runner.c
	x86_64-w64-mingw32-gcc -DCPLTESTS -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/breakpoint_hook.c -o bin/breakpoint_hook.o
	x86_64-w64-mingw32-gcc -DCPLTESTS -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/payload.c -o bin/payload.o
	java -Dcrystalpalace.verbose=false -classpath crystal-palace/crystalpalace.jar crystalpalace.spec.LinkSpec buildPic ./make_pico.spec x64 out/hwbp_pico.test.o
	java -Dcrystalpalace.verbose=false -classpath crystal-palace/crystalpalace.jar crystalpalace.spec.LinkSpec buildPic ./make_tester.spec x64 out/tester.bin
	
clean: bin out lib/libtcg/ lib/LibCPLTest/
	rm -f bin/*.o
	rm -f out/*.bin
	cd lib/libtcg && make clean
	cd lib/LibCPLTest && make clean
