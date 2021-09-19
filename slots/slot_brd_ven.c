#include "slots.h"
/*
 * slot_board_vendor_info(kind, slot, string, size)
 *	int kind
 *	int slot;
 *	char *string;
 *	int size;
 *
 * Slot_board_vendor_info returns the requested  vendor information
 * string into "string."  The request for "kind" is one of the vendor
 * information list types (found in slots.h.)  Size is the number of
 * bytes that "string" can hold.  Slot is the slot number or
 * access address that the vendor information is requested for.
 *
 * The return possibilites are zero, for no string found, -1, for
 * failure in access, and a positive integer for the number of bytes
 * successfully read.
 */

slot_board_vendor_info(kind, slot, string, size)
	int kind;		/* Vendor information type */
	int slot;		/* Slot number or ROM base address */
	char *string;		/* String buffer */
	int size;		/* The size of the buffer */
{
	struct rom_idoffset venlist[LISTLEN];
				/* The vendor list storage area */
	register char *romp;	/* A ROM pointer */
	register int i;		/* A loop counter */
	unsigned address;	/* The calculated ROM base address */

	/*
	 * Create the ROM base address
	 */
	if((address = slot_address(slot)) == 0)
	    return(-1);

	/*
	 * Get the RBL_VINFO (vendor information resource list) from
	 * the RD_BOARD (board resource) in the resource directory.
	 */
	if((romp = slot_resource(address, RD_BOARD, RBL_VINFO, 
			venlist, LISTLEN*sizeof(struct rom_idoffset))) == 0)
	    return(-1);
	/*
	 * Search the vendor information list for the "kind" item.
	 */
	for(i = 0; 
	    ((i < LISTLEN) && (venlist[i].r_id != RD_EOLIST));
	    i++) {
	    if(venlist[i].r_id == kind) 
		break;
	}
	if((i == LISTLEN) || (venlist[i].r_id == RD_EOLIST))
	    return(0);
	/*
	 * move romp forward in the structure so that it points at the
	 * the vendor list entry in the structure in ROM.
	 */
	romp = slot_calc_pointer(romp, i*sizeof(struct rom_idoffset));
	/*
	 * Then read the vendor information string requested.
	 */
	return(slot_structure(romp, venlist[i].r_offset, string, size));
}

