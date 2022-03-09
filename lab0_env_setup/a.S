.section ".text" // section "text"
_start:			// entry point	
	wfe			// wait for event (NOP operation)
	b _start	// branch operatoin (back to _start)
