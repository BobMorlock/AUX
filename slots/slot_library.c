/* 
 * Slot_library.c
 *
 * This file contains the minimum program text to support the Slot ROM
 * architecture described in the Slot Declaration Rom manager 
 * document.  The UNIX facilites required from a ROM manager do
 * not map directly to the structures and facilites described in
 * the Slot Declaration document.
 *
 * The differences between what is described in the document, and
 * what is required by the UNIX operating system may be reduced to a
 * the following:  UNIX requires a driver for each board, regardless
 * of the number of functions supported by the board.  The document
 * would like to remove the notion of a "board" and instead reference
 * functions, regardless of physical location.
 *
 * The routines found in this file are aimed at supporting a UNIX
 * type of operating system.  Only incedental support will be supplied
 * for the "boardless" concept of the world.
 */
#include <signal.h>
#include <setjmp.h>
#include "slots.h"

#ifdef TEST
#define slot_catch catch
#define slot_ignore ignore
#endif

/*
 * Global variables required by low level ROM accesses.
 */

jmp_buf slot_env;		/* Error protection slot_environment var */

/*
 * Interface routines.
 *
 * The following routines are provided to allow non-UNIX 
 * use of the slot manager routines.  In particular, slot_catch, and
 * slot_ignore are essentially renames of signal so that only two
 * routines have to be rewritten or replaced to make this file
 * non-UNIX specific.
 *
 * Slot_catch and slot_ignore are used when checks are made for board existance
 * in nubus slots.
 *
 * Slot_address is used to compute an access address for slot
 * ROM if necessary.
 *
 * Slot_seg_violation is called to handle errors.  
 */

/*
 * slot_seg_violation()
 *
 * Seg_violation is called when ROM fails to respond.  It long jumps
 * blindly to a location that has been set up earlier with a setjmp
 * call. 
 */
slot_seg_violation() {
	longjmp(slot_env, 0);
}

#ifndef TEST
/*
 * slot_catch(kind, routine)
 *	int kind;
 *	int routine();
 *
 * Slot_catch sets up the system to expect an interrupt of type "kind" and
 * call procedure "routine" if the interrupt is received.
 */
slot_catch(kind, routine)
	int kind;		/* Signal type */
	int routine;		/* Routine to call */
{
	signal(kind, routine);
}

/*
 * slot_ignore(kind)
 *	int kind;
 *
 * slot_ignore sets the system back to default handling of interrupts of
 * type "kind."
 */

slot_ignore(kind)
	int kind;		/* Signal type */
{
	signal(kind,SIG_DFL);
}
#endif

/*
 * unsigned slot_address(slot)
 *	int slot;
 *
 * Slot_address returns either a physical or virutal address
 * to be used to access ROM.  From the kernel, "slot" will contain
 * a physical address that may be used directly.  From user space,
 * slot will be a slot number that must be massaged into a virual
 * address and then "phys'd" in.
 */

unsigned slot_address(slot)
	int slot;		/* A slot number, or ROM base address */
{
	unsigned address;	/* The computed ROM base address */

	if((slot & 0xFF00000) == 0) {
	    /*
	     * The slot number passed in is just that, a slot number, and
	     * not an already valid address.  Phys the ROM into user accessable
	     * memory (at address 4Megs.)
	     */
	    address = 4*1024*1024;
	    if(phys(1,address,1024*1024,(u8 *)(0xf00f0000+(slot*0x1000000))) 
			!= 0) {
		return(0);
	    } else
		return(address);
	} else 
	    /*
	     * The slot number is really an address and may be used directly.
	     */
	    return(slot);
}

/*
 * char *slot_bytelane(address, bytelane)
 *	char *address;
 *	char *bytelane;
 *
 * bytelane returns find the byte lane information in a 
 * the ROM starting at base address "address."  The returned
 * pointer points to where the byte lane was found.  Bytelane
 * will have the byte lane nibble filled in.
 *
 * if there is not valid byte lane information, slot_bytelane returns
 * NULL (0.)
 */
char *slot_bytelane(base, bytelane)
	char *base;			/* ROM base address */
	char *bytelane;			/* A pointer to the byte lane area */
{
	register char *romp;		/* ROM pointer */
	register int i, j;		/* Loop counters */
	char bl = 0;			/* Temporary byte lane storage. */

	/*
	 * Create a starting pointer to search for the byte lane byte. Then
	 * search the last four byte address in ROM for a valid byte lane
	 * byte.
	 */
	for(i = 0,romp = (char *)((unsigned)base + 0xFFFF); i < 4; i++,romp--) {
	    /*
 	     * prepare for ROM access timeouts
	     */
	    slot_catch(SIGSEGV,  slot_seg_violation);
	    if(setjmp(slot_env) == 1) {
		slot_ignore(SIGSEGV); 
	        continue;
	    }
	    if((*romp & 0xf) == (~((*romp & 0xf0) >>  4) & 0xf))  { 
		/*
		 * The byte lane information looks good this far.
		 */
		bl = *romp & 0xf;
		/*
		 * Make sure that the bytelane byte was found at a valid
		 * address.
		 */
		if(((1 << ((unsigned)romp & 0x3)) & bl) != 0) {
		    *bytelane = bl;
		    break;
		}
	    }
	    /*
	     * Match the above slot_catch with an slot_ignore in the case of a
	     * non-trapped read, but invalid byte lane data.
	     */
	    slot_ignore(SIGSEGV);
	}
	/*
	 * No byte lane byte found in the last 4 bytes of ROM....
	 */
	if (i == 4) 
	    return((char *)0);
	/*
	 * Got a valid byte lane byte, restore trap handling and return
	 * a valid pointer.
	 */
	slot_ignore(SIGSEGV);
	return(romp);
}

/*
 * char *slot_calc_pointer(current, offset)
 *	char *current;
 *	int offset;
 *
 * Calc_pointer caluculates a pointer to a byte in the ROM, given the
 * pointer to the current byte, and an offset in bytes (to the new
 * byte if the memory was contiguous.)
 *
 * A NULL (0) pointer is returned in case of errors in the calculation, or
 * ROM accesses.
 */

char *slot_calc_pointer(curr, off)
	char *curr;		/* The current ROM access address */
	int off;		/* The required ROM offset in bytes */
{
	register int i;		/* A loop counter */
	int lanecount = 0;	/* The number of active byte lanes */
	int lowbits = 0;	/* The low three bits of the current ROM ptr */
	register int newoff = 0;
				/* A new calculated offset into ROM. */
	register int remainder = 0;
				/* The leftover bits in the address calcs */
	char bl;		/* The byte lane byte */

	/*
	 * Get the byte lane byte using the slot_bytelane routine.
	 * If no byte lane is available, we can't calculate the
	 * pointer.
	 */
	if(slot_bytelane(((unsigned)curr & 0xFFFF0000), &bl) == 0)
	    return(0);

	/*
	 * Test that the pointers bits 0 and 1 match the byte lane
	 * information. (BTST.B lowbits, bl)
	 */
	if(((1 << (lowbits = (unsigned)curr & 0x3)) & bl) == 0) 
	    return(0);

	/*
	 * Get the number of lanes active.
	 */	
	for(i = 0; i < 4; i++) 
	    lanecount += (bl >> i) & 0x1;
	if(lanecount == 0)
	    return(0);

	/*
	 * Sign extend a 24 bit negative offset if necessary.
	 */
	if((off & 0x800000) != 0)
	    off |= 0xff000000;

	/*
	 * Calculate the new pointer position using the lanecount.
	 */
	newoff = off / lanecount;
	remainder = off % lanecount;
	newoff = newoff * 4;
	/* 
	 * If there is a remainder, either back up or go forward the
	 * appropriate number of bytes to point the ROM pointer correctly.
	 */
	if(remainder != 0) { 
	    if(remainder < 0) {
		while(remainder < 0) {
		    newoff--;
		    if(lowbits-- < 0) lowbits = 3;
		    if(((1 << lowbits) & bl) != 0)
			remainder++;
		}
	    } else { 
		while(remainder > 0) {
		    newoff++;
		    lowbits = (lowbits + 1) % 4;
		    if(((1 << lowbits) & bl) != 0)
			remainder--;
		}
	    }
	}
	return(curr + newoff);
}

/*
 * slot_rom_data(address, width, data)
 *	caddr_t address;
 *	int	width;
 *	char	*data;
 *
 * Slot_rom_data returns information from board ROM into "data."  
 * Slot_rom_data is written to allow all of the other library 
 * routines to ignore the byte lane, word width issues, and simply 
 * return usable contiguous data.
 *
 * This routine will figure out the byte lane information each time it
 * is called, and access the ROM as specified in the format data.   This
 * is slow (a little bit,) but allows me not to have a global off
 * somewhere with the byte lane information stored.
 *
 * If an access fails, slot_rom_data returns -1, otherwise 
 * slot_rom_data returns the number of bytes read.
 *
 * "Width" is the number of bytes to be returned by slot_rom_data.   If
 * "width" is negative, then the bytes are read and stored in reverse
 * order. (Start large and work towards smaller addresses.) 
 */

slot_rom_data(address, width, data)
	unsigned address;		/* The ROM current access address */
	int	width;			/* The number of bytes (+ or -) to
					   be read */
	char	*data;			/* The data storage pointer */
{
	register char *romp;		/* A ROM pointer */
	register int count = 0;		/* The count of bytes read */
	char bl;			/* The byte lane byte */

	/*
	 * Get the byte lane byte using the upper 16 bits of the current
	 * access address to create the ROM base address.  If the access
	 * fails a negative -1 is returned.
	 */
	if(slot_bytelane(address & 0xFFFF0000, &bl) == 0)
	    return(-1);
	/*
 	 * prepare for ROM access timeouts
	 */
	slot_catch(SIGSEGV,  slot_seg_violation);
	if(setjmp(slot_env) == 1) {
	    slot_ignore(SIGSEGV);
	    return(-1);
	} else {
	    /*
	     * Calculate the address, and read the ROM bytes using "width"
	     * to decide what direction to read, and "bl" to decide whether
	     * the address is one that is accessible in ROM.
	     */
	    romp = (char *)address;
	    if(width < 0) {
		data -= width + 1;
		while(width < 0) {
		    if(((1 << ((unsigned)romp & 0x03)) & bl) != 0) {
			*data-- = (*romp & 0xff);
			width++;
			count++;
		    }
		    romp--;
		}
	    } else { /* width was positive */
		while(width > 0) {
		    if(((1 << ((unsigned)romp & 0x03)) & bl) != 0) {
			*data++ = (*romp & 0xff);
			width--;
			count++;
		    }
		   romp++;
		}
	    }
	}
	/*
	 * Put the interrupt handling back to its default, and return a
	 * positive number of bytes read.
	 */
	slot_ignore(SIGSEGV);
	return(count);
} 

/*
 * slot_check_crc(top, fhp, bytelane)
 *	char *top;
 *	struct format_header *fhp;
 *	char bytelane;
 *
 * Slot_check_crc uses the base and top address of ROM, and the information 
 * in the format_header to calculate a checksum.  
 *
 * If the checksum calculation fails, -1 will be returned.  0 indicates 
 * a good checksum.
 */
slot_check_crc(top, fhp, bytelane)
	char *top;			/* The largest ROM address */
	struct format_header *fhp;	/* The format header pointer */
	char bytelane;			/* The bytelane byte for ROM */
{
	register char *romp;		/* Current ROM pointer */
	register int length;		/* ROM size */
	long crc = 0, temp;		/* Temporary storage registers */
	int i;				/* Loop counter */
	char *crcmsb, *crclsb;		/* Crc most and least significant
					   addresses */

	/*
	 * Get the length from the format header, and the
	 * pointer to the base of ROM from top and length.
	 */
	length = fhp->f_length;
	if((romp = slot_calc_pointer(top, (-length) + 1)) == 0)
	    return(-1);
	crcmsb = slot_calc_pointer(top, 
		(unsigned)(&fhp->f_crc) - (unsigned)(&fhp->f_dataformat));
	crclsb = slot_calc_pointer(top, 
		(unsigned)(&fhp->f_rev) - (unsigned)(&fhp->f_dataformat));

	/*
	 * Prepare for ROM access failures.
	 */
	slot_catch(SIGSEGV,  slot_seg_violation);
	if(setjmp(slot_env) == 1) {
	    slot_ignore(SIGSEGV);
	    return(-1);
	}
	/*
	 * Calculate the CRC, making sure that the CRC bytes themselves
	 * are not added into the CRC sum.
	 */
	while(length > 0) {
	    if(((1 << ((unsigned)romp & 0x3)) & bytelane) != 0) {
		crc = (((crc & 0x80000000) >> 31) & 0x1) | (crc << 1);
		if((romp < crcmsb) || (romp >= crclsb)) {
		    temp = ((long)(*romp)) & 0xFF;
		    crc += temp;
		}
	 	length--;
	    }
	    romp++;
	}
	/*
	 * Go back to default interrupt handling.
	 */
	slot_ignore(SIGSEGV);
	/*
	 * Check that the calculated CRC matches the stored CRC.
	 */
	if(crc ==  fhp->f_crc) return(0);
	else return(-1);
}     

/*
 * char *slot_header(address, format_hdrp)
 *	char *address;
 *	struct format_header *format_hdrp;
 *
 * Slot_header returns the contents of the format header structure.
 * Slot_header has to decode the data width/byte lane information and
 * return the results in the structure pointed to by format_hdrp. Format_hdrp
 * points to the format_header structure as described in slots.h.  If
 * The slot requested either has no board installed, or does not have
 * properly formated ROM, then 0 is returned to flag the failure.
 * A pointer to the top of the format header in ROM is returned for the
 * non error case.
 *
 * "Address" is the address of the BASE of rom.  IE. for slot 9 "address"
 * will be F99F0000 if called from the kernel, ??0000 if called from
 * user space. (The ?? will be replaced by the megabyte virtual address
 * used in the phys call.)
 */
char *slot_header(base, fhp)
	char *base;			/* Rom base pointer */
	struct format_header *fhp;	/* Returned buffer pointer */
{
	register char *romp;		/* Rom pointer */
	char bytelane;			/* Byte lane nibble */
	int count;			/* count of bytes read */

	/*
	 * Get the byte lane byte, and a pointer to
	 * the location of the byte lane byte in ROM.  Zero is
	 * returned if no byte lane byte was found.
	 */
	romp = slot_bytelane(base, &bytelane);
	if(romp == (char *) 0) 
	    return(romp);

	/*
	 * Read the format header, again errors cause a zero return.
	 */
	if((count = 
		slot_rom_data(romp,-sizeof(struct format_header), fhp)) < 0)
	    return(0);

	/*
	 * Check that the format header contains valid information.
	 */
	if((fhp->f_testpattern != F_TESTPATTERN) || (fhp->f_rev > F_REV) ||
		(fhp->f_format != F_APPLE) || (fhp->f_reserved != 0) ||
		((fhp->f_diroffset & 0x00FFFFFF) == 0) ||
		((fhp->f_diroffset & 0xFF000000) != 0))
	    return(0);
	/* 
	 * If the checksum is bad, then also fail. 
	 */
	if(slot_check_crc(romp, fhp, bytelane) != 0)
	    return(0);

	/*
	 * Otherwise, return a pointer to the start of the format header
	 * in ROM.
	 */
	return(slot_calc_pointer(romp, (-count)+1));
}

