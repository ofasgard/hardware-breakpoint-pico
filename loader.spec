x64:
	# Load the runner PIC.
	load "bin/runner.o"
	make pic64 +optimize

	# Load and merge the two COFFs.
	load "bin/breakpoint_hook.o"
	make coff
	load "bin/payload.o"
	merge
	export
	
	# Make the merged COFF as a PICO and link to the PIC.
	make object
	export
	link "my_pico"
	
	# Export the whole thing.
	export
