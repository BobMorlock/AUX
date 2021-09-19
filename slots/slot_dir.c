#include "slots.h"
/*
 * char *slot_directory(address, dirp, size)
 *	caddr_t address;
 *	struct rom_idoffset dirp[];
 *	int size;
 *
 * Slot_directory returns the contents of the resource directory, up
 * to "size" entries, from ROM with a base address of "address."  Size is 
 * the number of resource entries will fit in the structure pointed to 
 * by "dirp."  The structure rom_idoffset is found in slots.h.  
 
 * Slot_directory returns a pointer to the beginning of the resource
 * directory if the ROM access and read is successful.  Null (0) is
 * returned in case of errors.
 */

char *slot_directory(address, dirp, size)
	unsigned address;		/* The ROM base address */
	struct rom_idoffset dirp[];	/* The list storage pointer */
	int size;			/* The size of the list */
{
	struct format_header fh;	/* A format header storage area */
	register char *romp;		/* The ROM pointer */

	/*
	 * Get the ROM header into "fh."  If no header is found, return
	 * a zero pointer.
	 */
	if((romp = slot_header(address, &fh)) == 0)
	    return(0);

	/*
	 * Calculate the pointer to the resource directory based on the
	 * address of the format_header in ROM and the offset for the
	 * directory found in the format header structure.
	 */
	romp = slot_calc_pointer(romp, fh.f_diroffset);

	/*
	 * Then read the resource directory into the area pointed to by
	 * dirp.  If the access fails, return zero, otherwise return a
	 * pointer to the start of the resource directory in ROM.
	 */
	if(slot_rom_data(romp, size * sizeof(int), dirp) < 0) 
	    return(0);
	return(romp);
}

