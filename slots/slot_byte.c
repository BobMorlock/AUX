#include "slots.h"
/*
 * u8 slot_byte(rbp)
 *	struct rsrc_byte *rbp;
 *
 * Slot_byte returns the byte contained in ROM which is part of
 * the rsrc_byte structure pointed to by "rbp."  The byte
 * returned will the LSB as decribed in the slot manager documentation.
 */
u8 slot_byte(rbp)
	struct rsrc_byte rbp;		/* The ROM rsrc_byte structure */
{
	return(rbp.rb_byte);
}

