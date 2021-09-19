/* Due to a BUG in the generic linker
   the size + length must not add up to 
   something with bit 31 set */
MEMORY {
	VALID: org = 0x500, len = 0x7FFFFAFF
}
SECTIONS {
	pstart 0x500: {
			nfspstart.o (.text, .data)
		}
	.text 0x10000000: {}
	.data 0x11000000: {}
	.bss  0x12000000: {}
	MODULES 0x0(COPY): {}
}
u = 0x3FC00000;
