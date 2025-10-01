name "Link the hardware breakpoint PICO with a simple PICO runner to make a blob of executable shellcode."
author "Calz0ne"

x64:
	# Load the runner PIC.
	load "bin/runner.o"
	make pic64 +optimize
	
	# Patch in ROR13 hashes for target function.
	patch "TARGET_MODULE_HASH" $TMH
	patch "TARGET_FUNCTION_HASH" $TFH

	# Load the HWBP pico and link it as "hwbp_pico".
	load "out/hwbp_pico.o"
	make object
	export
	link "hwbp_pico"
	
	# Export the whole thing.
	export
