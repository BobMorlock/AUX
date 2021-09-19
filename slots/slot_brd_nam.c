#include "slots.h"
/*
 * slot_board_name(slot, string, size)
 *	int slot;
 *	char string[];
 *	int size;
 *
 * Slot_board_name returns the board name string into "string," for
 * the board in slot "slot."
 *
 * A zero return indicates the string contents are valid.  A -1 is returned
 * if an error occured.
 */

slot_board_name(slot, string, size)
	int slot;		/* A slot number or a ROM base address */
	char string[];		/* The string where the name will be returned */
	int size;		/* The size of the string buffer */
{
	unsigned address;	/* The calculateed ROM base address */

	/* 
	 * Turn "slot" into a ROM access address.  Slot may either be
	 * a valid slot number, or an access address depending on the
	 * whether the caller is a kernel or user process.
	 */
	if((address = slot_address(slot)) == 0)
	    return(-1);

	/*
	 * Read the board name using the slot_resource routine.
	 */
	if(slot_resource(address, RD_BOARD, RL_NAME, string, size) == 0)
	    return(-1);
	return(0);
}

