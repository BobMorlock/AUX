#include "slots.h"
/*
 * char *slot_resource(address, kind, request, bp, size)
 *	caddr_t address;
 *	int kind;
 *	int request;
 *	char bp[];
 *	int size;
 *
 * Slot_resource returns "size" bytes of information into the
 * "bp" array.  The "kind" resource list will be searched for "request"
 * resource.  "Address" is the base of ROM in which the search will
 * be made.  
 * 
 * If any errors occur, a zero (0) will be returned.  If all goes well,
 * a pointer to the starting ROM address of the data will be
 * returned.
 */

char *slot_resource(address, kind, request, bp, size)
	unsigned address;		/* ROM base address */
	int kind;			/* The resource directory type */
	int request;			/* The resource list type */
	char bp[];			/* The storage buffer pointer */
	int size;			/* The size of the buffer  */
{
	struct rom_idoffset rl[LISTLEN];
					/* The resource list storage area */
	register char *romp;		/* A ROM pointer */
	register int i;			/* A loop counter */

	/*
	 * Get the resource list for the resource of type kind found in
	 * the resource directory.
	 */
	if((romp = slot_resource_list(address, kind, rl, LISTLEN)) == 0)
	    return(0);
	/*
	 * Search the list for the resource information of type request.
	 */
	for(i = 0; 
	    ((i < LISTLEN) && (rl[i].r_id != RD_EOLIST));
	    i++) 
	    if(rl[i].r_id == request) 
		break;
	
	if((i == LISTLEN) || (rl[i].r_id == RD_EOLIST))
	    return(0);
	/*
	 * move romp forward in the structure so that it points at the
	 * the requested entry in the resource list in ROM.
	 */
	romp = slot_calc_pointer(romp, i*sizeof(struct rom_idoffset));
	/*
	 * Then fill in "bp" using the slot_structure routine.
	 */
	if(slot_structure(romp, rl[i], bp, size) < 0)
	    return(0);
	/*
	 * Return the address in ROM of the requested resource.
	 */
	return(slot_calc_pointer(romp, rl[i].r_offset));
}

