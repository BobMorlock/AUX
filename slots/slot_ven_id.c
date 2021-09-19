#include "slots.h"
/*
 * slot_vendor_id(slot, string, size)
 *	int slot;
 *	char *string;
 *	int size;
 *
 * Slot_vendor_id returns the Vendor ID string from the board in slot
 * "slot."  
 *
 * A negative value is returned in case of error.  A zero
 * is returned if the string is null.  A positive number is returned if
 * one or more characters are now in the buffer pointed to by "string."
 * The positive number is the count of characters copied.
 */

slot_vendor_id(slot, string, size)
	int slot;		/* A slot number or ROM base address */
	char *string;		/* A string buffer pointer */
	int size;		/* The size of the string buffer */
{
	/*
	 * Get the board vendor information of type B_VID (board vendor
	 * id.)
	 */
	return(slot_board_vendor_info(B_VID, slot, string, size));
}

