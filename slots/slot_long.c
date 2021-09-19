#include "slots.h"
/*
 * u32 slot_long(address, rd)
 *	struct rom_idoffset rd;
 *	caddr_t address;
 *
 * Slot_long returns 32 bits of information contained in ROM  
 * pointed to by "rd."  The word returned is the 32 bits pointed to rlp
 * but offset by "address."  Longs are found by using an address, plus an 
 * offset to point to a full 32 bit quantum.  Bytes and words are
 * stored directly, this convention is described in the
 * slot manager documentation.  What this means is that slot_long
 * has one more address calculation done before the ROM access than
 * either of the byte or word ROM accesses.
 */

u32 slot_long(address, rdp)
	struct rom_idoffset rdp;	/* Long pointer */
	unsigned address;		/* Long pointers ROM address */
{
	register char *romp;		/* A ROM pointer */
	long value;			/* The long value to be readd */
	register int count;		/* The number of bytes read */

	/*
	 * Calculate a pointer to the long data in ROM.
	 */
	romp = (char *)address;
	romp = slot_calc_pointer(romp, rdp.r_offset);
	/*
	 * Read the long data checking for errors, and  return the value
	 * read.
	 */
	if((count = slot_rom_data(romp, sizeof(long), &value)) != sizeof(long))  
	    return(0xFFFFFFFF);
	return(value);
}

