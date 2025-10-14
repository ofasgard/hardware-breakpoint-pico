name "Make the hardware breakpoint PICO by merging hardware_breakpoint.o and payload.o"
author "Calz0ne"

x64:
	# Load the COFF that contains the entrypoint and breakpoint hooking logic.
	load "bin/breakpoint_hook.o"
	make coff
	
	# Merge in the COFF that contains the payload.
	load "bin/payload.o"
	merge
	
	# Export the result as a combined COFF file.
	export
