#include "slots.h"
/*
 * slot_PRAM_init(slot, Pp)
 *	int slot;
 *	struct PRAM *Pp;
 *
 * Slot_PRAM_init fills in the PRAM structure from the ROMs at
 * slot "slot."  
 *
 * A negative value is returned in case of error.  A zero
 * is returned if Pp now points to a valid structure.
 */

slot_PRAM_init(slot, Pp)
	int slot;		/* A slot number or ROM base address */
	struct PRAM *Pp;	/* The PRAM structure pointer */
{
	unsigned address;	/* The calculated ROM base address */

	/*
	 * Get the ROM base address
	 */
	if((address = slot_address(slot)) == 0)
	    return(-1);
	
	/*
	 * Get the board resource list item of type RBL_PRAM from the
	 * resource list of type RD_BOARD in the resource directory.
	 */
	if(slot_resource(address, RD_BOARD, RBL_PRAM, 
			Pp, sizeof(struct PRAM)) == 0)
	    return(-1);
	return(0);
}

