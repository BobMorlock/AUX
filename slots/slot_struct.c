#include "slots.h"
/*
 * slot_structure(address, from, to, size)
 *	caddr_t address;
 *	struct rom_idoffset from;
 *	char *to;
 *	int size;
 *
 * Slot_structure is the low level routine that moves structures from
 * ROM to RAM.  Address is the address in ROM where the structure
 * should reside.  "From" is the resource offsett, and "to" is where the
 * the structure should be put.  Size is the number of bytes that are to
 * be moved.
 *
 * A negative return indicates that the copy failed.  A positive return
 * indicates success, and is the number of bytes moved.
 */

slot_structure(address,from,to,size)
	struct rom_idoffset from;	/* The ROM from pointer */
	unsigned address;		/* The ROM base address */
	char *to;			/* The RAM pointer */
	register int size;		/* The size of buffer in RAM */
{
	register char *romp;		/* A ROM pointer */

	/*
	 * Calculate the pointer to the structure in ROM based on "from"
	 * and the ROM base address "address."  
	 */
	romp = (char *)address;
	romp = slot_calc_pointer(romp,from.r_offset);
	/*
	 * Then read size bytes to "to" from "romp," using the
	 * read rom data routine.
	 */
	return(slot_rom_data(romp, size, to));  
}

