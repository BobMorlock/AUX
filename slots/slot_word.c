#include "slots.h"
/* 
 * u16 slot_word(rwp)
 *	struct rsrc_word rwp;
 *
 * Slot_word returns 16 bits of information contained in ROM  
 * in the structure pointed to by rwp.  The word returned is the LSW as 
 * described in the slot manager documentation.
 */
u16 slot_word(rwp)
	struct rsrc_word rwp;
{
	return(rwp.rw_word);
}

