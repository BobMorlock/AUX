/* NFSSRC @(#)conf.h	2.1 86/04/11 */
/*	@(#)conf.h	UniPlus VVV.2.1.1	*/
/*
 *	Declaration of block device switch. Each entry (row) is
 *	the only link between the main unix code and the driver.
 *	The initialization of the device switches is in the file conf.c.
 */
extern	struct	bdevsw	{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_strategy)();
	int	(*d_print)();
} bdevsw[];

/*
 *	Character device switch.
 */
extern	struct	cdevsw	{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_read)();
	int	(*d_write)();
	int	(*d_ioctl)();
	struct tty *d_ttys;
	int	(*d_select)();
	struct streamtab *d_str;
} cdevsw[];

extern	int	bdevcnt;
extern	int	cdevcnt;

/*
 *	Streams modules 
 */
 
#define FMNAMESZ	8

struct fmodsw {
	char f_name[FMNAMESZ+1];
	struct streamtab *f_str;
};

extern struct fmodsw fmodsw[];

extern int fmodcnt;

/*
 *	Line discipline switch.
 */
extern	struct	linesw	{
	int	(*l_open)();
	int	(*l_close)();
	int	(*l_read)();
	int	(*l_write)();
	int	(*l_ioctl)();
	int	(*l_input)();
	int	(*l_output)();
	int	(*l_mdmint)();
} linesw[];

extern	int	linecnt;
