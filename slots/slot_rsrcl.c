#include "slots.h"

/*
 * char *slot_resource_list(address, kind, listp, size)
 *	caddr_t address;
 *	struct rom_idoffset *listp;
 *	int size;
 *
 * Slot_resource_list returns the contents of the resource directory, up
 * to "size" entries from ROM with a base address of "address."  "Size" is 
 * the number of resource entries will fit in the structure pointed to 
 * by "listp."  The structure rom_idoffset is found in slots.h. 
 *
 * Slot_resource_list will return zero if there is an error.
 * The resource list information from ROM will be returned into the
 * buffer pointed to by listp and a pointer to the beginning of the 
 * resource list will be returned if there are no errors.
 *
 */

char *slot_resource_list(address, kind, listp, size)
	unsigned address;		/* The ROM base address */
	int kind;			/* The resource directory type */
	struct resource_board_list *listp;
					/* The buffer pointer */
	int size;			/* The size in bytes of the buffer */
{
	struct rom_idoffset rd[LISTLEN];
					/* Resource directory storage area */
	register char *romp;		/* A ROM pointer */
	register int i;			/* A loop counter */

	/*
	 * Get the resource directory into "rd."
	 */
	if((romp = slot_directory(address, rd, LISTLEN)) == 0)
	    return(0);
	/*
	 * Search the list for the requested  resource.
	 */
	for(i = 0; ((i < LISTLEN) && (rd[i].r_id != RD_EOLIST)); i++) {
	    if(rd[i].r_id == kind) break;
	}
	if((i == LISTLEN) || (rd[i].r_id == RD_EOLIST))
	    return(0);
	
	/*
	 * "i" now points to the board resource item, so use it to
	 * calculate the location of the board resource list, and
	 * then read the data.
	 */
	romp = slot_calc_pointer(romp, i * sizeof(struct rom_idoffset));
	romp = slot_calc_pointer(romp, rd[i].r_offset);
	if(slot_rom_data(romp, 
		size * sizeof(struct rom_idoffset), listp) < 0)
	    return(0);
	return(romp);
}
