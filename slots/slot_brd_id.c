#include "slots.h"
/*
 * u16 slot_board_id(slot)
 *	int slot;
 *
 * Slot_board_id returns the unique board number assigned by the apple
 * board number guru for the boad in slot "slot."
 * 
 * In case of errors, an INVALID id will be returned (0xffff.)
 */

u16 slot_board_id(slot)
	int slot;		/* A slot number or a ROM base address */
{
	int data;		/* The location into which the board id data
				   from ROM will be stored. */

	if(slot_data(slot, RD_BOARD, RBL_BOARDID, &data, 16) < 0)
	    return(0xffff);
	return(data & 0xffff);
}

