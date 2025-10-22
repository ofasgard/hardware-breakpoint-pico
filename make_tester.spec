name "Link the hardware breakpoint PICO with a simple PICO runner to make a blob of executable shellcode."
author "Calz0ne"

x64:
	# Load the runner PIC.
	load "bin/runner.o"
	make pic +optimize +gofirst
	
	# Merge in LibTCG for PICO loading and function resolution.
	mergelib "lib/libtcg/libtcg.x64.zip"
	
	# Opt into dynamic function resolution.
	dfr "resolve" "ror13"

	# Load the HWBP pico.
	load "out/hwbp_pico.test.o"
	make object
	
	# Merge in libraries.
	mergelib "lib/libtcg/libtcg.x64.zip"
	mergelib "lib/LibCPLTest/libcpltest.x64.zip"
	
	# Export and link as "hwbp_pico".
	export
	link "hwbp_pico"
	
	# Export the whole thing.
	export
