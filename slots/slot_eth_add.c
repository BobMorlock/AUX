#include "slots.h"
/*
 * slot_ether_addr(slot, bytes)
 *	int slot;
 *	char *bytes;
 *
 * Slot_ether_addr returns the ethernet address string from the 
 * board in slot "slot."  
 *
 * Slot_ether_addr reurns a negative value if the access fails. 
 * The number of bytes of ethernet address read is returned when the 
 * read is successfull. 
 */

slot_ether_addr(slot, bytes)
	int slot;		/* A slot number or ROM base address */
	char *bytes;		/* A pointer to the 6 byte address buffer */
{
	unsigned address;	/* The calculated ROM base address */

	/*
	 * Calculate  the ROM base address.
	 */
	if((address = slot_address(slot)) == 0)
	    return(-1);

	/*
	 * Read the resource list type item RL_ETHERADDR from the resource
	 * directory type RD_ETHER resouce structure.  If the access is
	 * unsuccessfull, return a negative value, otherwise return
	 * 6 as the standard number of ethernet address bytes.
	 */
	if(slot_resource(address, RD_ETHER, RL_ETHERADDR, bytes, 6) == 0)
	    return(-1);
	return(6);
}

