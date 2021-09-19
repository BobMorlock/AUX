/*								*/
/* NCR5380 NCR5380 NCR5380 NCR5380 NCR5380 NCR5380 NCR5380	*/
/* NCR5380 NCR5380 NCR5380 NCR5380 NCR5380 NCR5380 NCR5380	*/
/* NCR5380 NCR5380 NCR5380 NCR5380 NCR5380 NCR5380 NCR5380	*/
/* NCR5380 NCR5380 NCR5380 NCR5380 NCR5380 NCR5380 NCR5380	*/
/* NCR5380 NCR5380 NCR5380 NCR5380 NCR5380 NCR5380 NCR5380	*/

struct	ncr5380 {
	unsigned char	curr_data;	/* R   Current SCSI data */
	unsigned char	fill1[15];
	unsigned char	init_comm;	/* R/W Initiator Command */
	unsigned char	fill2[15];
	unsigned char   mode;		/* R/W Mode		 */
	unsigned char	fill3[15];
	unsigned char 	targ_comm;	/* R/W Target Command	 */
	unsigned char	fill4[15];
	unsigned char	curr_stat;	/* R   Current Status	 */
	unsigned char	fill5[15];
	unsigned char	bus_status;	/* R   Bus and Status bits*/
	unsigned char	fill6[15];
	unsigned char   in_data;	/* R   Input Data	 */
	unsigned char	fill7[15];
	unsigned char	respar_int;	/* R   Reset Parity/Ints */
	};
#define		out_data curr_data	/* W   Output Data 	 */
#define		sel_ena	 curr_stat	/* W   Select Enable	 */
#define		start_xmt bus_status	/* W   Start DMA SCSI xmit */
#define		start_Trcv in_data	/* W   Start Target SCSI rcv */
#define		start_Ircv respar_int	/* W   Start Initiator SCSI rcv */

/*	Initiator Command register bits */

#define	SIC_DB	0x01	/* Data bus */
#define	SIC_ATN 0x02	/* ATN line */
#define	SIC_SEL	0x04	/* SEL line */
#define	SIC_ACK	0x10	/* ACK line */
#define	SIC_RST	0x80	/* Reset line */

/*	Mode register bits */

#define	SMD_DMA	0x02	/* DMA transfer enable */

/*	Target Command Register bits

#define	STC_IO	0x01
#define	STC_CD	0x02
#define	STC_MSG	0x04
#define	STC_REQ	0x08

/*	Current status register bits */

#define	SCS_REQ	0x20	/* REQ line */
#define	SCS_BSY	0x40	/* BSY line */

/*	Bus status register bits */

#define	SBS_PHASE 0x08	/* Phase match */
#define	SBS_DMA	  0x40	/* DMA request bit */

/*	States for Phases */
/*	Derive state from MSG, SEL, I/O */

#define	SPH_DOUT 0x00	/* Data out phase */
#define	SPH_DIN	0x01	/* Data in phase */
#define	SPH_CMD	0x02	/* Command phase */
#define	SPH_STAT 0x03	/* Status phase */
#define	SPH_MOUT 0x06	/* Message out phase */
#define	SPH_MIN	0x07	/* Message in phase */

/*	Here is a handy generic form of a group zero command block.
 */

struct scsig0cmd {
	u_char op;		/* 0: opcode */
	u_char addrH;		/* 1: logical address 2 and LUN */
	u_char addrM;		/* 2: logical address byte 1*/
	u_char addrL;		/* 3: logical address byte 0 */
	u_char len;		/* 4: number of blocks or bytes of other data */
	u_char ctl;		/* 5: control field */
	};
#define	SCIG0LEN 6


/*	Some common SCSI command op codes */

#define	SOP_RDY		0x00	/* test unit ready */
#define	SOP_SENSE	0x03	/* sense error information */
#define	SOP_FMT		0x04	/* format the device */
#define	SOP_READ	0x08	/* short form group 0 read */
#define	SOP_WRITE	0x0A	/* short form group 0 write */
#define	SOP_INQ		0x12	/* inquiry command */
#define	SOP_SELMODE	0x15	/* set device mode information */
#define SOP_GETMODE	0x1A	/* sense/get mode information */
#define	SOP_READCAP	0x25	/* read capacity of device */

/*	Some SCSI message codes */

#define	SMG_COMP	0x00	/* Command complete */
#define	SMG_DISC	0x04	/* Disconnect */
#define	SMG_ABT		0x06	/* Abort */
#define	SMG_RESET	0x0C	/* Reset bus device */

/*	The following data structure is used to pass requests
 *	to the low level SCSI manager routines.  Disk driver routines need
 *	not know about SCSI protocol.  The driver routines specify to the SCSI
 *	manager everything that is required for a transfer, control returns
 *	to the driver either when the transfer is complete, or an error
 *	has occurred.
 */

struct	scsireq {
	caddr_t cmdbuf;		/* buffer containing command block */
	caddr_t	databuf;	/* buffer containing the data to send or rcv */
	unsigned datalen;	/* length of the data buffer */
	unsigned datasent;	/* length of data actually sent or recieved */
	caddr_t	sensebuf;	/* optional error result from sense cmd */
	int	(*faddr)();	/* address of completion function */
	long	driver;		/* private storage for driver placing request */
	u_char	cmdlen;		/* length of command buffer */
	u_char	senselen;	/* optional length of the sense buffer */
	u_char	sensesent;	/* length of sense data recieved */
	char	flags;		/* request flag bits */
	u_char	msg;		/* mode byte returned on cmd complete */
	u_char	stat;		/* completion status byte */
	u_char	ret;		/* return code from UNIX SCSI software */
	};

/*	Request flag bits.
 */

#define	SRQ_READ 1 		/* This request reads data */
#define	SRQ_NOTIME 2		/* This request will not time out */

/*	SCSI Status Codes.
 *	Return codes from scsireq().
 *	Device drivers should cope easily with errors > SST_FATAL
 *	error codes < SST_FATAL may indicate problems with the SCSI bus.
 */

#define	SST_BSY 1	/* SCSI bus was busy, stayed busy, and is still busy */
#define	SST_CMD	2	/* An error happened during command transmission */
#define	SST_COMP 3	/* an error occured during the status phase */
#define SST_SENSE 4	/* An error occured obtaining sense data */
#define	SST_SEL	5	/* No unit reponded to the given id selection */
#define	SST_TIMEOUT 6	/* Request took longer than 5 seconds */
#define	SST_MULT 7	/* Driver(s) placed multiple requests for this id */
#define	SST_FATAL 7
#define	SST_MORE 8	/* There was more data than the device expected */
#define	SST_LESS 9	/* There was less data than the device expected */
#define	SST_STAT 10	/* Status word was non zero, sense command executed */

/*	SCSI device characteristics.
 *	     These are used in calls to scsidevchar() to indicate the
 *	characteristics of a device.
 */

#define	SDC_532	1	/* device is 532 byte blocked */

#ifdef	KERNEL
extern	char *scsi_strings[];
#endif	KERNEL

#ifdef	STANDALONE
#define SCSI_ADDR	0x50F10000
#define	SDMA_ADDR	0x50014000	/* SCSI special DMA register */
#endif	STANDALONE
