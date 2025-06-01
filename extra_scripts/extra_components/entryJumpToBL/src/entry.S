.section	.init,"ax",@progbits
.global	_start
.align	1
_start:
	// Directly jump to the Boot Loader (BL) at a specific address
	// The address is 0x6D000, which is the target of the jump
	j	0x6D000
