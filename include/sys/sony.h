/* header file for sony driver */

/* disk address offsets from base */

#define Ph0L	0x200*0
#define Ph0H	0x200*1
#define Ph1L	0x200*2
#define Ph1H	0x200*3
#define Ph2L	0x200*4
#define Ph2H	0x200*5
#define Ph3L	0x200*6
#define Ph3H	0x200*7
#define MtrOff	0x200*8
#define MtrOn	0x200*9
#define IntDrv	0x200*10		/* enable internal drive address */
#define ExtDrv	0x200*11		/* enable external drive address */
#define Q6L	0x200*12
#define Q6H	0x200*13
#define Q7L	0x200*14
#define Q7H	0x200*15

/* absolute addresses */

#define DPh0L	IWM_ADDR + Ph0L
#define DPh0H	IWM_ADDR + Ph0H
#define DMtrOff	IWM_ADDR + MtrOff
#define DMtrOn	IWM_ADDR + MtrOn
#define DiskQ6L	IWM_ADDR + Q6L
#define DiskQ6H	IWM_ADDR + Q6H
#define DiskQ7L	IWM_ADDR + Q7L
#define DiskQ7H	IWM_ADDR + Q7H

/* data values for setting IWM for various operations */

#define DirLAdr		0
#define DirHAdr		1
#define DIPAdr		2		/* Disk In Place test address */
#define StepLAdr	4
#define MtrOnAdr	8
#define MtrOffAdr	9
#define Tk0Adr		10
#define DrvExstAdr	0xD
#define TachAdr		0xE
#define WrProtAdr	6

#define RdDataAdr	1
#define RdData1Adr	3
#define SidesAdr	9
#define EjectHAdr	0xD

#define SNCNT		2

#define CA2		1
#define SEL		2
#define CA0		4
#define CA1		8

#define	S_OFF		0
#define S_INPROG	1
#define S_DONE		2
