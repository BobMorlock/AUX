#include "slots.h"
/*
 * u16 slot_board_flags(slot)
 *	int slot;
 *
 * Slot_board_flags returns the board flags for the board located in
 * slot "slot."  Flag bit definitons may be found in slots.h.
 *
 * The only possible error return is all bits set, if the board access
 * failed.
 */

u16 slot_board_flags(slot)
	int slot;		/* A slot number, or a ROM base address */
{
	int data;		/* The location into which the flag data 
				   information is stored. */

	if(slot_data(slot, RD_BOARD, RBL_FLAG, &data, 16) < 0)
	    return(0xffff);
	return(data & 0xffff);
}

