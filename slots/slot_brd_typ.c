#include "slots.h"
/*
 * int slot_board_type(slot, data)
 *	int slot;
 *	char *data;
 *
 * Slot_board_type returns the board type as an unsigned 64 bit quantity 
 * for the board found in slot "slot."
 *
 * A zero return indicates no error. -1 is returned
 * if an error occured.
 */

int slot_board_type(slot, data)
	int slot;		/* A slot number or a ROM base address. */
	char data[];		/* Where the type is to be stored. */
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
	 * Read the 8 byte board type into the data buffer.
	 */
	if(slot_resource(address, RD_BOARD, RL_TYPE, data, 8) == 0)
	    return(-1);
	return(0);
}

