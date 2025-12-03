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

	# Load the HWBP pico and link it as "hwbp_pico".
	load "out/hwbp_pico.o"
	make object +optimize
	export
	link "hwbp_pico"
	
	# Export the whole thing.
	export
