/*
 * Slot manager structure definitions.
 */

/*
 * Useful "generic" definitions.
 */

typedef unsigned char     u8;
typedef unsigned short   u16;
typedef unsigned int     u32;

/* 
 * Slot Format-Header block.
 */

struct format_header {
  long	f_diroffset;		/* Directory Offset, relative dir-off */
  long	f_length;		/* Length used when computing the checksum */
  long	f_crc;			/* Checksum */
  char	f_rev;			/* Rev level of the ROM format. */
#define F_REV		9  	/* Current apple formate rev level */
  char	f_format;		/* One of 256 unique ROM formats. */
#define F_APPLE		1	/* Apple format, any other value is vendor
				 * specific.
				 */
  long	f_testpattern;		/* A predefined test pattern */
#define F_TESTPATTERN	0x5a932bc7

  char	f_reserved;		/* Reserved for future use. */
  char	f_dataformat;		/* This is a special nibble in one of the
				 * bytes of the first 4 addressable byes that
				 * specifies the data format of the rom.
				 */
};

#define	DIR_SIZE	1	/* A Directory is at least one entry long */
#define LIST_SIZE	DIR_SIZE

/*
 * Slot Resource Directory definitions.
 */
#define RD_APPLE_LO	0
#define RD_APPLE_HI	127
#define RD_VENDOR_LO	128
#define RD_VENDOR_HI	245
#define	RD_EOLIST	255
/*
 * Resource types
 */
#define RD_BOARD	1
#define RD_ETHER	128
#define RD_TST1		129

/*
 * The "standard" ROM "pointer" structure.
 */
struct rom_idoffset {
      long	r_id: 8;	/* Resource ID */
      long	r_offset: 24;	/* Resource Offset */
};

/*
 * Data formats.
 */
struct rsrc_byte {		/* A byte of data */
    long	rb_id: 8;	/* Resource ID */
    long	rb_zero: 16;
    long	rb_byte: 8;	/* The byte of data */
};

struct rsrc_word {		/* A word of data */
    long	rw_id: 8;	/* Resource ID */
    long	rw_zero: 8;
    long	rw_word: 16;	/* the word of data */
};

#define BLK_SIZE	1024	/* An example block size */

struct rsrc_blk {
    long	rblk_size;	/* How long the block really is */
    long	rblk_data[BLK_SIZE];
				/* The block of data */
};

/*
 * Slot resource type.  This structure is required in a resource list
 * and is pointed to when the resource id == RL_TYPE.
 */

struct rsrc_type {
    unsigned short	rtyp_cat;	/* The resource category */
    unsigned short	rtyp_typ;	/* The resource type */
    unsigned short	rtyp_sw;	/* The software type */
    unsigned short	rtyp_hw;	/* The hardware type */
};

/*
 * sResource types
 */
/*
 * Category
 */
#define R_CAT_BOARD	1
#define R_CAT_TEST	2
#define R_CAT_DISP	3
#define R_CAT_NET	4

/*
 * Type
 */
#define R_TYP_BOARD	0
#define R_TYP_APPLE	1
#define R_TYP_VIDEO	1
#define R_TYP_ETHER	1
/*
 * Driver Interface <id.SW>
 */
#define R_DRSW_APPLE	1

/*
 * Driver Interface <id.HW>
 */
#define R_DRHW_TFB	1
#define R_DRHW_GM	2
#define R_DRHW_3COM	1

union resource {
    struct rom_idoffset r_real;
    struct rsrc_byte	r_byte;		/* Just byte of data */
    struct rsrc_word 	r_word;		/* Just a word of data */
    char		*r_string;	/* A string pointer */
    long		*r_long;	/* A long pointer */
    struct rsrc_blk	*r_blk;		/* A block pointer */
    struct rsrc_list	*r_list;	/* A list pointer */
    struct rsrc_type 	*r_type;	/* A resource type pointer */
    struct rsrc_blk	*r_drvr;	/* A pointer to a list of drvrs */
};


/* 
 * Slot Resource Lists.
 * 
 * The definitions found here are common to ALL resources.
 */
/*
 * Required Items.
 */
#define	RL_TYPE		1		/* Type of slot resource */
#define RL_NAME		2		/* Name of slot resource */
#define RL_EOLIST	RD_EOLIST

/*
 * Optional Items.
 */
#define RL_ICON		3		/* Icon */
#define RL_DRVR		4		/* Driver directory */
#define RL_LDREC	5		/* Load record */
#define RL_BTREC	6		/* sBoot record */
#define RL_FLAG		7		/* sResource flags */
#define RL_HWDID	8		/* Hardware device ID */
#define RL_MINBOS	10		/* Offset and length of sResource */
#define RL_MINLEN	11		/* in minor space */
#define RL_MAJBOS	12		/* Offset and length of sResource */
#define RL_MAJLEN	13		/* in major space */
#define RL_DRVRDIR	16		/* sDriver directory */

/*
 * Test Resource list types.
 */
#define RL_BYTE		0x20
#define RL_WORD		0x21
#define RL_LONG		0x22
#define RL_STRING 	0x23
#define RL_ETHERADDR	0x80

struct rsrc_list {
    union resource r_list[LIST_SIZE];
};

/*
 * Optional DRVR directory structure.  This structure is pointed to by
 * the resource list offset when the resource id == RL_DRVRDIR.
 */

struct rsrc_drvr {
  struct rsrc_blk drvr_list[LIST_SIZE];
};

#define DRVR_68000	1		/* Code for a Mac OS, 68000 */
#define DRVR_68020	2		/* Code for a Mac OS, 68020 */
#define DRVR_EOLIST	RL_EOLIST
#define DRVR_CODE_SIZE	1000		/* An example number */
#define DRVR_HDR_SIZE	400		/* Another example */


  
/*
 * Board resource list structure.
 *
 * For slot resource lists of CATAGORY board, there are additional fields
 * defined.  The following structure is used in BOARD case only.
 * rsrc_type.rtyp_cat == R_CAT_BOARD, rsrc_type.rtyp_typ == R_TYP_BOARD
 */
union resource_board_list {
    union resource rbl_resource;	/* All of the original definitions. */
    struct	PRAM	 *rbl_pram;	/* PRAM INIT data pointer */
    struct	priminit *rbl_init;	/* Primary init record */
};

/*
 * Resource list types for the BOARD catagory only.
 */

#define RBL_TYPE	RL_TYPE
#define RBL_NAME	RL_NAME
#define	RBL_BOARDID	32		/* rsrc_word structure */
#define RBL_PRAM	33
#define RBL_PRIM	34
#define RBL_SECND	35
#define RBL_VINFO	36
#define RBL_FLAG	37

/*
 * Board flag definitions
 */
#define FLAG_SIM	1
#define FLAG_NOTSIM	~FLAG_SIM

/*
 * Board Resource List structure definitions.
 */

struct PRAM {
    long	pram_size;		/* Block size (8 bytes, 6 used) */
    char	pram_zero[2];		/* Must be zero */
    char	pram_bytes[6];		/* bytes 1 through 6 of PRAM data */
};

#define CODE_SIZE	1024		/* Example size for a block of code */

struct priminit {
    long prim_size;			/* Block size */
    char prim_rev;			/* Revision level */
    char prim_cpu;			/* CPU ID */
    short prim_timeout;			/* Init time expected */
    long prim_offset;			/* offset, relative to code */
    char prim_code[CODE_SIZE];		/* The init code */
};


/* 
 * Vendor information list types.
 */

#define	B_VID	1			/* Vendor ID */
#define B_SN	2			/* Serial Number */
#define B_RL	3			/* Revision Level */
#define B_PN	4			/* Part number */


/*
 * Useful definitions...
 */
#define LISTLEN		20		/* The "standard" maximum list size */

/*
 * Library routine names that return character pointers.
 */

char 	*slot_calc_pointer();		/* Calculates a new ROM pointer */
char	*slot_resource_list();		/* Returns a pointer to a rsr list */
char	*slot_resource();		/* Returns a pointer to a rsr struct */
char	*slot_bytelane();		/* Returns a pointer to the byte */
char	*slot_header();			/* Returns a pointer to the ROM hdr */
char	*slot_directory();		/* Returns a pointer to the rsr dir */


/*
 * Board IDs.
 */

#define ETHER_3COM_ID	8
