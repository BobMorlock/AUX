#include "slots.h"
/* 
 * slot_part_num(slot, string);
 *	int slot;
 *	char *string;
 *
 * Slot_part_num returns the part number string from the board in slot
 * "slot."  
 *
 * A negative value is returned in case of error.  A zero
 * is returned if the string is null.  A positive number is returned if
 * one or more characters are now in the buffer pointed to by "string."
 * The positive number is the count of characters copied.
 */

slot_part_num(slot, string, size)
	int slot;		/* A slot number or ROM base address */
	char *string;		/* The storage string pointer */
	int size;		/* The size of the string buffer */
{
	/*
	 * Get board vendor information of type B_PN (board part number.)
	 */
	return(slot_board_vendor_info(B_PN, slot, string, size));
}

