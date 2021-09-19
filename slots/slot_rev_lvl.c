#include "slots.h"
/* 
 * slot_rev_level(slot, string);
 *	int slot;
 *	char *string;
 *
 * Slot_rev_level returns the revision level string from the board in slot
 * "slot."  
 *
 * A negative value is returned in case of error.  A zero
 * is returned if the string is null.  A positive number is returned if
 * one or more characters are now in the buffer pointed to by "string."
 * The positive number is the count of characters copied.
 */

slot_rev_level(slot, string, size)
	int slot;		/* A slot number or ROM base address */
	char *string;		/* The string storage pointer */
	int size;		/* The size of the string buffer */
{
	/*
	 * Get board vendor information of the B_RL (board rev level.)
	 */
	return(slot_board_vendor_info(B_RL, slot, string, size));
}

