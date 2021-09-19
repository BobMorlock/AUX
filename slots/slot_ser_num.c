#include "slots.h"
/*
 * slot_serial_num(slot, string, size)
 *	int slot;
 *	char *string;
 *	int size;
 *
 * Slot_serial_num returns the serial number string from the board in slot
 * "slot."  
 *
 * A negative value is returned in case of error.  A zero
 * is returned if the string is null.  A positive number is returned if
 * one or more characters are now in the buffer pointed to by "string."
 * The positive number is the count of characters copied.
 */

slot_serial_num(slot, string, size)
	int slot;		/* A slot number or ROM base address */
	char *string;		/* A string buffer pointer */
	int size;		/* The size of the buffer */
{
	/*
	 * Get the board vendor information of type B_SN (board serial 
	 * number.)
	 */
	return(slot_board_vendor_info(B_SN, slot, string, size));
}

