#include "slots.h"
/*
 * slot_primary_init(slot, primp)
 *	int slot;
 *	struct priminit *primp;
 *
 * slot_primiary_init fills in the priminit structure from the ROMs at
 * slot "slot."  
 *
 * A negative value is returned in case of error.  A zero
 * is returned if priminit is now valid.
 */

slot_primary_init(slot, primp)
	int slot;		/* A slot number, or ROM base address */
	struct priminit *primp; /* The storage area pointer */
{
	unsigned address;	/* The calculated ROM base address */

	/*
	 * Calculate the ROM base address.
	 */
	if((address = slot_address(slot)) == 0)
	    return(-1);

	/*
	 * Read the resource list item RBL_PRIM from the resource directory
	 * item RD_BOARD type.
	 */
	if(slot_resource(address, RD_BOARD, RBL_PRIM, 
			primp, sizeof(struct priminit)) == 0)
	    return(-1);
	return(0);
}

