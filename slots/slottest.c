#include <stdio.h>
#include "slots.h"


main()
{
	char string[100];
	register int i, j, k, m;
	register int result;
	unsigned address;
	struct format_header fh, *fhp;
	char *romp;
	char bytelane;
	int count;
	char tstchar;
	u16 tstshort;
	u32 tstlong;
	struct rom_idoffset rd[20];
	struct rom_idoffset rl[20];
	struct rsrc_type rtype;

	for(i = 9; i < 15; i++) {
	    printf("\n\nSlot %d:\n", i);
	    /* 
	     * This is NOT what user code should do.  I am just seeing what
	     * is out in ROM.
	     */
	    /* 
	     * Create an address and phys in the ROM.
	     */
	    address = slot_address(i);
	    /*
	     * Get the byte lane byte, and a pointer to
	     * the location of the byte lane byte in ROM.  Zero is
	     * returned if no byte lane byte was found.
	     */
	    romp = slot_bytelane(address, &bytelane);
	    if(romp == (char *) 0) {
		printf("Couldn't read the bytelane byte.\n");
		continue;
	    }
	    /*
	     * Read the format header, again errors cause a zero return.
	     */
	    fhp = &fh;
	    if((count = 
		slot_rom_data(romp,-sizeof(struct format_header), fhp)) < 0) {
		    printf("Couldn't read the format header\n");
		    continue;
	    }
	    /*
	     * Check that the format header contains valid information.
	     */
	    if((fhp->f_testpattern != F_TESTPATTERN) || (fhp->f_rev > F_REV) ||
	       (fhp->f_format != F_APPLE) || (fhp->f_reserved != 0) ||
	       ((fhp->f_diroffset & 0x00FFFFFF) == 0) ||
	       ((fhp->f_diroffset & 0xFF000000) != 0)) {
		   printf("it is bad\n");
		   continue;
	    }
	    /* 
	     * If the checksum is bad, then also fail. 
	     */
	    if(slot_check_crc(romp, fhp, bytelane) != 0) {
		printf("Bad checksum\n");
		continue;
	    }
	    
	    /*
	     * Otherwise, return a pointer to the start of the format header
	     * in ROM.
	     */
	    romp = slot_calc_pointer(romp, (-count)+1);
	    
	    /*
	     * We have a valid board... so go ahead.
	     */
	    if((romp = slot_directory(address, rd, 20)) == 0) {
		printf("Can't get the resource directory\n");
		continue;
	    }
	    for(j = 0; ((j < 20) && (rd[j].r_id != RD_EOLIST)); j++) {
		printf("	Resource dir[%d]: {<%x>,<%X>}\n", j, 
			rd[j].r_id, rd[j].r_offset & 0xffffff);
		slot_resource(address, rd[j].r_id, RL_TYPE, &rtype, 
			      sizeof(struct rsrc_type));
		printf("	Resource type:	%x %x %x %x\n",
		       rtype.rtyp_cat, rtype.rtyp_typ, rtype.rtyp_sw,
		       rtype.rtyp_hw);
		printf("	Resource List:\n");
		if((romp = 
		    slot_resource_list(address, rd[j].r_id, rl, 20)) == 0) {
			printf("Can't get the resource list\n");
			break;
		}
		for(k = 0; ((k < 20) && (rl[k].r_id != RL_EOLIST)); k++) {
		    printf("			%d	%x	%X\n", k, 
			   rl[k].r_id, rl[k].r_offset & 0xffffff);
		}
		printf("\n");
		if(rd[j].r_id == RD_TST1) {
		    printf("	Test Resource contents:\n");
		    if((romp = 
			slot_resource(address, RD_TST1, 
				      RL_STRING, string, 80)) == 0)
			printf("	No resource STRING.\n");
		    else
			printf("	resource STRING: %s\n", string);
		    if(slot_data(address, 
				 RD_TST1, RL_LONG, &tstlong, 32) < 0)
			printf("	No resource LONG\n");
		    else
			printf("	resource LONG:	 %X\n", tstlong);
		    if(slot_data(address, RD_TST1, 
				 RL_WORD, &tstlong, 16) < 0)
			printf("	No resource SHORT.\n");
		    else
			printf("	resource SHORT:	 %x\n", tstlong&0xffff);
		    if(slot_data(address, 
				 RD_TST1, RL_BYTE, &tstlong, 8 ) < 0)
			printf("	No resource BYTE.\n\n");
		    else
			printf("	resource BYTE:	 %x\n\n", tstlong&0xff);
		}
	    }

	    if((result = slot_board_id(i)) == 0xffff) {
		printf("	Board access failed\n");
		continue;
	    }
	    printf("	Board ID:	%x\n", result);
	    if(result == ETHER_3COM_ID) {
	        if(slot_ether_addr(i, string) > 0)
		    printf("	Net address:	%.2x %.2x %.2x %.2x %.2x %.2x\n",
			string[0] & 0xff, string[1] & 0xff, string[2] & 0xff,
			string[3] & 0xff, string[4] & 0xff,
			string[5] & 0xff);
	        else
		    printf("	No ether net address on the ethernet board!\n");
		if(slot_data(address, RD_ETHER, RL_MINBOS, &tstlong, 32) < 0)
		    printf("	No CSR address\n");
		else
		    printf("	CSR address:	%X\n", tstlong);
	    }
	    if(slot_board_name(i, string, 100) == 0)
	        printf("	Board name:	%s\n", string);
	    else
		printf("	No board name found\n");
	    if((slot_board_type(i, &rtype)) == 0)
	        printf("	Board type:	%x %x %x %x\n",
		       rtype.rtyp_cat, rtype.rtyp_typ, rtype.rtyp_sw,
		       rtype.rtyp_hw);
	    else
		printf("	No board type found\n");
	    if(slot_vendor_id(i, string, 100) > 0)
	        printf("	Vendor ID:	%s\n", string);
	    else
		printf("	No vendor ID found\n");
	}
}
	    
