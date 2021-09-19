#include "slots.h"
/*
 * slot_data(slot, kind, request, datap, size)
 *	int slot;
 *	int kind;
 *	int request,
 *	int *datap;
 *	int size;
 *
 * Slot_data reads "size" bits of data into "datap" from the
 * the resource list of type "kind" for resource of type "request" for
 * the board in slot "slot."  Slot_data is the low level ROM
 * access routine used by many of the other slot manager functions.
 * 
 * Slot_data returns -1 if an error occured.  If the data
 * is valid, a zero is returned.
 */

slot_data(slot, kind, request, datap, size)
	int slot;		/* A slot number or ROM base address */
	int kind;		/* One of the resource list types in slots.h */
	int request;		/* One of the resource list sub types */
	int *datap;		/* A pointer to where the data storage area */
	int size;		/* The size in bits of the expected data */
{
	struct rom_idoffset rl[LISTLEN];
				/* A storage area for a resource list */
	register char *romp;	/* A pointer into ROM */
	register int i;		/* A loop counter */
	unsigned address;	/* The ROM base address */

	/*
	 * Create the ROM base address from "slot." 
	 */
	if((address = slot_address(slot)) == 0)
	    return(-1);

	/*
	 * Get the resource list if there is one for the board in slot
	 * "slot."
	 */
	if((romp = slot_resource_list(address, kind, rl, LISTLEN)) == 0)
	    return(-1);
	/*
	 * Search the resource list for the board resource requested.
	 */
	for(i = 0; ((i < LISTLEN) && (rl[i].r_id != RD_EOLIST)); i++) {
	    if(rl[i].r_id == request) 
		break;
	}

	/*
	 * If the board resource requested wasn't found, return an error
	 */
	if((i == LISTLEN) || (rl[i].r_id == RD_EOLIST))
	    return(-1);

	/*
	 * Depending on the number of bits to be read, call the
	 * appropriate access routine.
	 */
	switch(size) {
	case 8:
	    *datap = slot_byte(rl[i]);
	    return(0);
	case 16:
	    *datap = slot_word(rl[i]);
	    return(0);
	case 32:
	    if((romp = 
		slot_calc_pointer(romp, i*sizeof(struct rom_idoffset))) == 0)
		return(-1);
	    *datap = slot_long(romp, rl[i]);
	    return(0);
	default:
	    return(-1);
	}
}

