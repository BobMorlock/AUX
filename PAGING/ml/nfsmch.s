	file	"nfsmch.s"
 #	@(#)nfsmch.s	UniPlus VVV.2.1.12

#define LOCORE
#include	"mch.h"
#include "sys/param.h"
#include "sys/page.h"
#include "sys/uconfig.h"
#include "sys/sysmacros.h"
#include "sys/mmu.h"
#include "sys/seg.h"
	# USIZE dependencies - assembler does not understand '<<'
#if PAGESHIFT==PS2K
#define	NBPP		2048
#endif
#if PAGESHIFT==PS4K
#define	NBPP		4096
#endif
#if PAGESHIFT==PS8K
#define	NBPP		8192
#endif
#if PAGESHIFT==PS16K
#define	NBPP		16384
#endif
#if PAGESHIFT==PS32K
#define	NBPP		32768
#endif	PAGESHIFT
	set	UDOTSIZE%,NBPP*USIZE	# Size of U area (bytes)

	# Configuration dependencies
	set	HIGH%,0x2700		# High priority supervisor mode (spl 7)
	set	LOW%,0x2000		# Low priority, supervisor mode (spl 0)

	global	u,utblbp
	data	
	set	utblbp,UDOT+UDOTSIZE%

	global kstack%, splimit%, m20cache, mmu_on, ivecstart, kstart, mmuaddr
	global fp881

	even
m20cache:	long	0
mmu_on:         long    0               # Is the mmu enabled yet
cputype%:	short	0		# Local copy of _cputype, 0 if 68000
splimit%: 	long	0		# For generic m68k stack probe mechanism
kstack%:	long	0		# temporary stack pointer
ivec%:		long	0		# temp area
fvec%:		long	0		# temp area
ivecstart:	long	VECBASE		# interrupt vector base address
kstart:		long	KSTART		# starting kernel virtual address
mmuaddr:	long	MMUADDR		# CPU address of mmu
fp881:		short	0		# true if we have an 881

	# MC68881 Floating Point Coprocessor Data
fpnull%:	long	0		# Format word for MC68881 Null State

	text	
	global	_start,end,edata,main,cputype,ivect
#ifdef AUTOCONFIG
	global	sectinfo
#endif AUTOCONFIG

_start:	mov.w	&HIGH%,%sr		# spl7
#ifdef AUTOCONFIG
	mov.l	sectinfo+28,%d7		# start of bss
	add.l	sectinfo+32,%d7		# + size = end of bss
	add.l	&UDOTSIZE%+NBPP-1,%d7	# End of unix
	and.l	&-NBPP,%d7		# Round to nearest click
	mov.l	sectinfo+28,%a0		# Start clearing here (start of bss)
clrbss%:mov.l	&0,(%a0)+		# Clear bss
	cmp.l	%a0,%d7
	bcs	clrbss%
#else 
	mov.l	&end+UDOTSIZE%+NBPP-1,%d7	# End of unix
	and.l	&-NBPP,%d7		# Round to nearest click

	mov.l	&edata,%a0		# Start clearing here
clrbss%:mov.l	&0,(%a0)+		# Clear bss
	cmp.l	%a0,%d7
	bcs	clrbss%
#endif AUTOCONFIG
 	mov.l	%d7,%sp			# Move off the booter's stack

 # Determine cpu type
	mov.l	KSTART+16,ivec%		# Illegal instruction vector
	mov.l	KSTART+44,fvec%		# F-line Emulator vector
	mov.l	&LS%1,KSTART+16		# Illegal instruction vector
	mov.l	&KSTART,%d0		# Load interrupt vector base register
	short	0x4E7B			#	movec
	short	0x0801			#	d0,vbr
	mov.l	&68010,cputype		# No trap. Must be 68010 or 68020
	mov.w	&1,cputype%		# Local copy
	mov.w	&0x3700,%sr		# Set master state
	mov.w	%sr,%d0
	mov.w	&0x2700,%sr		# Restore interrupt state
	btst	&12,%d0			# Master state set in 68020 only
	beq	LS%1			# Not set - it's a 68010
	mov.l	&68020,cputype		# Must be 68020
	mov.w	&2,cputype%		# Local copy

	mov.l	&CACHEON,%d0		# turn on on-chip cache
	mov.l	%d0,m20cache		# save if cache is disabled/enabled
	or.l	&CACHECLR,%d0		# invalidate all entries in cache
	#	movecl	d0,cacr
	short	0x4e7b
	short	0x0002
	#	movecl	d0,cacr		# repeat required due to bug on 68020
	short	0x4e7b
	short	0x0002
	nop				# required due to bug on 68020
	#	movecl	d0,cacr		# repeat required due to bug on 68020
	short	0x4e7b
	short	0x0002

#ifndef NOFPU
	# Determine if MC68881 Floating Point Coprocessor is present, by
	#	attempting to issue a reset (restore null state)
	# Note: this only makes sense if we know there's a 68020
	mov.l	&LS%1,KSTART+44		# F-line Emulator vector
	mov.l	&fpnull%,%a0		# a0 points to null state format word
	short	0xF350			#	frestore (a0)
	mov.w	&1,fp881		# No trap, mc68881 found as CP-ID 1
#endif NOFPU

LS%1:	
	mov.l	ivec%,KSTART+16		# restore Illegal instruction vector
	mov.l	fvec%,KSTART+44		# F-line Emulator vector
	mov.l	%d7,%sp			# Stack for mmualloc call
	mov.l	%d7,-(%sp)
	jsr	mmualloc		# Allocate space for mmu tables
	add.l	&4,%sp
	mov.l	%d0,%d7			# Save end of tables (tblend)
	jsr	mmuinit			# Initialize mmu
	mov.l	&memerror%,KSTART+8	# set up for memory buserr
 	mov.l	%d7,-(%sp)		# save a copy of %d7 on the stack
 	mov.l	%sp, kstack%		# 	save the stack
	mov.l	%d7,-(%sp)		# arg to memsize - where to start
	jsr	memsize			# memory sizer (physical memory)

memerror%:
 	mov.l	kstack%, %sp		# restore our stack/%d7 state
 	mov.l	(%sp)+, %d7		# in case memsize used %d7 and
 					# then trapped to memerror%
 	mov.l	%sp, %a0		# push the address of the current udot,
 	sub.l	&UDOTSIZE%, %a0		# 	it will become the idle udot
 	mov.l	%a0, -(%sp)		#	eventually
	mov.l	%d7,-(%sp)		# tblend where to allocate tables
	jsr	vadrspace		# vadrspace(tblend), init returns here
	mov.l	&utblbp,%sp		# Set stack at end of U area
	mov.l	%sp,kstack%		# vaddrof stackend (resched exit)
	sub.l	&4,kstack%		# in case used before decrement
	jsr	main			# Long jump to unix, init returns here

	tst.l	%d0
	beq	LS%20
	mov.l	%d0,%a0
	jmp	(%a0)

LS%20:
	clr.l	-(%sp)			# Indicate short 4 byte stack format
	mov.l	&USTART,-(%sp)		# Starting program address
	clr.w	-(%sp)			# New sr value
	rte				# Call init

	# save and restore of register sets

	global	save,resume,qsave,uptbl
save:	mov.l	(%sp)+,%a1		# return address
	mov.l	(%sp),%a0		# ptr to label_t
	movm.l	&0xFCFC,(%a0)		# save d2-d7, a2-a7
	mov.l	%a1,48(%a0)		# save return address
	mov.l	&0,%d0
	jmp	(%a1)			# return

qsave:	mov.l	(%sp)+,%a1		# return address
	mov.l	(%sp),%a0		# ptr to label_t
	add.w	&40,%a0
	mov.l	%fp,(%a0)+		# save a6
	mov.l	%sp,(%a0)+		# save a7
	mov.l	%a1,(%a0)+		# save return address
	mov.l	&0,%d0
	jmp	(%a1)		# return

	# resume(save area, proto page table)
resume:	
#ifdef FLOAT
 	jsr	fpresume		# save the state of fp and COPYSEG
#endif
#ifdef mc68881
 	jsr	fpsave			# save the state of fp and COPYSEG
#endif
	mov.l	4(%sp),%a0		# ptr to udot's label_t
	mov.l	8(%sp),%a1		# address of local udot pg tbl
	mov.w	&HIGH%,%sr		# spl 7

	mov.l	uptbl,%a3
#ifdef	MMB
	mov.l	&8-1,%d0
#else	MMB
	mov.l	&USIZE-1,%d0
#endif	MMB
LS%89:	mov.l	(%a1)+,(%a3)+
	dbra	%d0,LS%89
#ifdef	PMMU
#ifdef	PMMUMMB
	#       pflusha                 # flush all atc entries
	short   0xF000
	short   0x2400
#else	PMMUMMB
	#       pflusha                 # flush all atc entries, for now
	short   0xF000
	short   0x2400
#endif	PMMUMMB
#else	PMMU
	mov.l	&0x7,%d0		# flush ATC by loading root pointer
	#	movecl	d0,dfc
	short	0x4e7b
	short	0x0001
	mov.l	rootbl,%d0
	mov.l	&MMUADDR,%a1		# write to root pointer
	#	movesl	d0,a1@
	short	0x0e91
	short	0x0800
#endif	PMMU
	mov.l	m20cache,%d0		# set enable/disable bit for 68020 CACR
	or.l	&CACHECLR,%d0		# flush 68020 cache
	#	movecl	d0,cacr
	short	0x4e7b
	short	0x0002
	#	movecl	d0,cacr		# repeat required due to bug on 68020
	short	0x4e7b
	short	0x0002
	nop				# required due to bug on 68020
	#	movecl	d0,cacr		# repeat required due to bug on 68020
	short	0x4e7b
	short	0x0002
	nop

	mov.l	44(%a0),%sp		# restore the stack
	mov.l	%a0,%a3
#ifdef CLR_CACHE
	mov.l	&1,-(%sp)
	jsr	clr_cache
	add.l	&4,%sp
#endif CLR_CACHE
	jsr	ldustbl
	mov.l	%a3,%a0
	movm.l	(%a0)+,&0xFCFC		# restore the registers
	mov.l	(%a0),%a1		# fetch the original pc
	mov.l	&1,%d0			# return 1
	mov.w	&LOW%,%sr		# set spl0
	jmp	(%a1)			# return

	# Reset the MC68881 Floating-point Coprocessor
	# by restoring the null state.
	global	fpreset
fpreset:mov.l	&fpnull%,%a0		# a0 points to null state format word
	short	0xF350			#	frestore (a0)
	rts	

	text	
	# spl commands
	global	splhi, splx
	global	splclock
	global	splimp, splnet
	global	spl7, spl6, spl5, spl4, spl3, spl2, spl1, spl0

splhi:
spl7:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2700,%sr		# set priority 7
	rts	
splclock:
splimp:
splnet:
spl6:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2600,%sr		# set priority 6
	rts	
spl5:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2500,%sr		# set priority 5
	rts	
spl4:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2400,%sr		# set priority 4
	rts	
spl3:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2300,%sr		# set priority 3
	rts	
spl2:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2200,%sr		# set priority 2
	rts	
spl1:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2100,%sr		# set priority 1
	rts	
spl0:	mov.w	%sr,%d0			# fetch current CPU priority
	mov.w	&0x2000,%sr		# set priority 0
	rts	

splx:	mov.w	6(%sp),%sr		# set priority
	rts	

	data	
	global	idleflg
idleflg:short	0

	text	
	global	qrunflag, queueflag, stream_run
	global	idle,idle1%,waitloc
idle:
	mov.w	%sr,-(%sp)		# Fetch current CPU priority
	mov.w	&1,idleflg		# Set idle flag
	mov.w	&0x2700, %sr		# Do streams scheduling (crit. section)
	tst.b	qrunflag		# Are there streams to do?
	beq	LQR%11
	tst.b	queueflag		# Are they already being done?
	bne	LQR%11
	mov.b	&1, queueflag		# OK. Do them
	mov.w	&0x2000, %sr		# Exit crit. section
	mov.l	stream_run, %a0		
	jsr	(%a0)			# Call streams scheduler
	clr.b	queueflag		# Tidy up on the way out
LQR%11:
	mov.w	&0x2000,%sr		# Set priority zero
idle1%:	tst.w	idleflg			# Wait for interrupt
	bne	idle1%
waitloc:			# Pseudo location addr used by kernel profiling
	mov.w	(%sp)+, %sr	# Restore priority
	rts	
#define ASADEBUG
#ifdef ASADEBUG
	global	idle1
idle1:	mov.w	%sr,%d0
	mov.w	&1,idleflg
	mov.w	&0x2000,%sr
idle11%:tst.w	idleflg	
	bne	idle11%
	mov.w	%d0,%sr
	rts	
	global	idle2
idle2:	mov.w	%sr,%d0
	mov.w	&1,idleflg
	mov.w	&0x2000,%sr
idle12%:tst.w	idleflg
	bne	idle12%
	mov.w	%d0,%sr
	rts	
#endif ASADEBUG

	global	buserr%, addrerr%, fault%, call%, busaddr
	global	runrun, trap

fault%:
	clr.w	-(%sp)			# this makes ps long aligned
	clr.w	idleflg			# clear idle flag
	movm.l	&0xFFFF,-(%sp)		# save all registers
	btst	&5,70(%sp)		# did we come from user mode?
	bne	LS%100			# no, don't trash kernel sp
	mov.l	%usp,%a0
	mov.l	%a0,60(%sp)		# save usr stack ptr
LS%100:
	mov.l	66(%sp),%d0		# return ptr from the jsr
	sub.l	&ivect+4,%d0		# subtract ivect table offset
	asr.l	&2,%d0			# calculate vector number
resched%:	mov.l	%d0,-(%sp)	# argument to trap
	jsr	trap			# C handler for traps and faults
	add.l	&4,%sp
	bsr	checksir%		# check for software interrupt requests
	btst	&5,70(%sp)		# did we come from user mode?
	bne	LS%4			# no, just continue
	mov.w	&0x2700, %sr		# Do streams scheduling (crit. section)
	tst.b	qrunflag		# Are there streams to do?
	beq	LQR%1
	tst.b	queueflag		# Are they already being done?
	bne	LQR%1
	mov.b	&1, queueflag		# OK. Do them
	mov.w	&0x2000, %sr		# Exit crit. section
	mov.l	stream_run, %a0		
	jsr	(%a0)			# Call streams scheduler
	clr.b	queueflag		# Tidy up on the way out
	bra	LQR%2
LQR%1:	mov.w	&0x2000, %sr
LQR%2:
	tst.l	runrun			# should we reschedule?
	beq	LS%4			# no, just return normally
	mov.l	&256,%d0		# 256 is reschedule trap number
	bra	resched%		# go back into trap

LS%4:
	btst	&5,70(%sp)		# did we come from user mode?
	bne	LS%103			# no, don't trash kernel sp
	mov.l	60(%sp),%a0
	mov.l	%a0,%usp		# restore usr stack ptr
LS%103:
	movm.l	(%sp)+,&0x7FFF		# restore all other registers
	add.w	&10,%sp			# sp, pop fault pc, and alignment word
	rte	

	global	syscall0%, syscall0, syscall1%, syscall1

syscall0%:
	clr.w	-(%sp)			# this makes ps long aligned
	movm.l	&0xFFFF,-(%sp)		# save all registers
	mov.l	%usp,%a0
	mov.l	%a0,60(%sp)		# save usr stack ptr
	btst	&5,70(%sp)		# did we come from user mode?
	bne	LS%5			# no, error !!!
	jsr	syscall0		# Process system call
	bsr	checksir%		# check for software interrupt requests
	mov.w	&0x2700, %sr		# Do streams scheduling (crit. section)
	tst.b	qrunflag		# Are there streams to do?
	beq	LQR%3
	tst.b	queueflag		# Are they already being done?
	bne	LQR%3
	mov.b	&1, queueflag		# OK. Do them
	mov.w	&0x2000, %sr		# Exit crit. section
	mov.l	stream_run, %a0		
	jsr	(%a0)			# Call streams scheduler
	clr.b	queueflag		# Tidy up on the way out
	bra	LQR%4
LQR%3:	mov.w	&0x2000, %sr
LQR%4:
	tst.l	runrun			# should we reschedule?
	beq	LS%6			# no, just return normally
	mov.l	&256,%d0		# 256 is reschedule trap number
	bra	resched%		# go back into trap

syscall1%:
	clr.w	-(%sp)			# this makes ps long aligned
	movm.l	&0xFFFF,-(%sp)		# save all registers
	mov.l	%usp,%a0
	mov.l	%a0,60(%sp)		# save usr stack ptr
	btst	&5,70(%sp)		# did we come from user mode?
	bne	LS%5			# no, error !!!
	jsr	syscall1		# Process system call
	bsr	checksir%		# check for software interrupt requests
	mov.w	&0x2700, %sr		# Do streams scheduling (crit. section)
	tst.b	qrunflag		# Are there streams to do?
	beq	LQR%5
	tst.b	queueflag		# Are they already being done?
	bne	LQR%5
	mov.b	&1, queueflag		# OK. Do them
	mov.w	&0x2000, %sr		# Exit crit. section
	mov.l	stream_run, %a0		
	jsr	(%a0)			# Call streams scheduler
	clr.b	queueflag		# Tidy up on the way out
	bra	LQR%6
LQR%5:	mov.w	&0x2000, %sr
LQR%6:
	tst.l	runrun			# should we reschedule?
	beq	LS%6			# no, just return normally
	mov.l	&256,%d0		# 256 is reschedule trap number
	bra	resched%		# go back into trap


LS%5:	movm.l	(%sp)+,&0x7FFF		# restore registers
	add.l	&6,%sp		# sp, pop fault pc, and alignment word
	mov.l	&ivect+132,(%sp)		# simulate a bsr
	bra	fault%

LS%6:	mov.l	60(%sp),%a0
	mov.l	%a0,%usp		# restore user stack pointer
	movm.l	(%sp)+,&0x7FFF		# restore all other registers
	add.w	&10,%sp			# sp, pop fault pc, and alignment word
	rte	

	# Bus error entry, this has its stack somewhat different.  We will
	# call a C routine to save the info then fix the stack to look like
	# a trap.  These entries will be called directly from interrupt vector.

buserr%:
	# hardflt(): 	if successful vfault() or pfault()
	#			return 0
	#		else
	#			return -1
	jsr	fl_atc			# clear bus error by flushing ATC

 #------------------------------------------------------------------------
 # The following code performs a patch to fix up the bus error stack.  The 020
 # has several bugs which cause it to write the bus error stack incorrectly.
 # The original version of this patch assumed that sp pointed directly to the
 # bus error frame.  Since it is necessary to save some registers before
 # executing the patch, the stack offsets have been adjusted.

	mov.l	%d0,-(%sp)

	mov.l	runtime,%d0
	and.l	&RT_A23G+RT_A92E,%d0
	beq	skipcode		# check if correction needed

	cmp.w	(8+0x06)(%sp),&0xB008	# check for long stack frame
	bne	skipcode

	mov.w	(8+0x36)(%sp),%d0	# check for bad internal register
	cmp.w	%d0,&0x591
	beq	super
	cmp.w	%d0,&0x1D7
	beq	super
	cmp.w	%d0,&0x1DF
	beq	super
	cmp.w	%d0,&0x1DD
	beq	super
	cmp.w	%d0,&0x0B6
	beq	super

	cmp.w	%d0,&0x1D6		# last stage dbcc
	beq	dbccfix			# replace register 20

	mov.w	(8+0x1C)(%sp),%d0
	and.w	&0xFFC0,%d0
	cmp.w	%d0,&0x6C0		# call or return module
	beq	norecover

	mov.l	(8+0x24)(%sp),%d0
	cmp.l	%d0,(8+0x28)(%sp)
	beq	fixitem8
	add.l	&2,%d0
	mov.l	%d0,(8+0x20)(%sp)
	bra	fixitem8

dbccfix:mov.l	(8+0x14)(%sp),(8+0x20)(%sp)
	bra	skipcode

fixitem8:
	cmp.w	(8+0x36)(%sp),&0x5FB	# sync mode problem
	bne	skipcode

	btst	&2,(8+0xB)(%sp)	# medium stack
	bne	super

	bclr	&5,(8+0x0)(%sp)

super:	mov.w	(8+0x1C)(%sp),(8+0x58)(%sp) # long to medium stack
	mov.l	(8+0x18)(%sp),(8+0x54)(%sp)
	mov.l	(8+0x14)(%sp),(8+0x50)(%sp)
	mov.l	(8+0x10)(%sp),(8+0x4C)(%sp)
	mov.l	(8+0x0C)(%sp),(8+0x48)(%sp)
	mov.l	(8+0x08)(%sp),(8+0x44)(%sp)

	bclr	&4,(8+0x6)(%sp)
	mov.l	(8+0x4)(%sp),(8+0x40)(%sp)
	mov.l	(8+0x0)(%sp),(8+0x3C)(%sp)

	mov.l	(4+0x0)(%sp),(4+0x3C)(%sp)	# move saved bsr
	mov.l	(%sp),(0x3C)(%sp)		# move saved d0

	lea	(0x3C)(%sp),%sp

skipcode:
	mov.l	(%sp)+,%d0		# restore d0

 # End of stack fixup.
 #------------------------------------------------------------------------
	clr.w	-(%sp)			# this makes ps long aligned
	movm.l	&0xFFFF,-(%sp)		# save all registers
	btst	&5,70(%sp)		# did we come from user mode?
	bne	LS%101			# no, don't trash kernel sp
	mov.l	%usp,%a0
	mov.l	%a0,60(%sp)		# save usr stack ptr
LS%101:
	cmp.w	cputype%,&2
	beq	LS%60			#
	mov.w	78(%sp),%d0		# 68010 Special Status Word
	ext.l	%d0
	mov.l	80(%sp),%d1		# 68010 Fault Address
	bra	LS%70
LS%60:					# 68020
	#jsr	fl_atc			# clear bus error by flushing ATC
	#mov.w	80(%sp),%d0		# 68020 Special Status Word
	#ext.l	%d0
	#mov.l	%sp,%d0			# fault frame address
	#mov.l	86(%sp),%d1		# 68020 Fault Address
LS%70:
	#mov.l	%d0,-(%sp)		# Special Status Word
	#mov.l	%d1,-(%sp)		# Fault Address
	jsr	hardflt			# hardflt(vaddr, &lbuserr)
	#add.l	&8,%sp
	tst.l	%d0
	bne	norecover		# call trap
	btst	&5,70(%sp)		# did we come from user mode?
	bne	LS%102			# no, don't trash kernel sp
	mov.l	60(%sp),%a0
	mov.l	%a0,%usp		# restore user stack pointer
LS%102:
	movm.l	(%sp)+,&0x7FFF		# restore all other registers
	add.l	&10,%sp			# pop bsr and old usp
	rte				# successful vfault()/pfault(), so rte

norecover:
					# see if stack cleanup is needed
					# (user handler may not have used last
					# buserr frame after catching SIGSEGV)
	btst	&5,70(%sp)		# did we come from user mode?
	bne	LS%114			# no, just continue
	cmp.w	cputype%,&1		# get size of stack frame in d0
	bne	LS%106
	mov.l	&58+70,%d0		# size of 68010 stack frame + regs
	bra	LS%110
LS%106:					# 68020 stack frame
	btst	&4,76(%sp)		#   is this a long buserr stack frame?
	bne	LS%108
	mov.l	&32+70,%d0		#   size of short buserr frame + regs
	bra	LS%110
LS%108: mov.l	&92+70,%d0		#   size of long buserr frame + regs
LS%110:
	mov.l	&UDOT+UDOTSIZE%-8,%a0	# stack bottom (8 bytes fudge factor)
	sub.l	%d0,%a0			# top of first buserr frame
	cmp.l	%sp,%a0			# is stack clean up needed
	bhi     LS%114			#   if not, get out

	mov.l	%sp,%a0
	add.l	%d0,%a0			# source copy location
	mov.l	&UDOT+UDOTSIZE%-4,%a1	# destination copy location
LS%112:	mov.b	-(%a0),-(%a1)		# stack frame copy loop
	cmp.l	%a0,%sp
	bhi	LS%112
	mov.l	%a1,%sp			# adjust stack pointer
	nop			#debug
	nop			#debug
	nop			#debug

LS%114:					# make stack look like we're coming
 					#   from lfault%
	btst	&5,70(%sp)		# did we come from user mode?
	bne	LS%104			# no, don't trash kernel sp
	mov.l	60(%sp),%a0
	mov.l	%a0,%usp		# restore user stack pointer
LS%104:
	movm.l	(%sp)+,&0x7FFF		# restore all other registers
	add.l	&6,%sp			# pop usp
LS%9:
	mov.l	&ivect+12,(%sp)		# simulate a bsr
	bra	fault%

addrerr%:
	mov.l	&ivect+16,(%sp)		# simulate a bsr
	bra	fault%

	# common interrupt dispatch

call%:	
	movm.l	&0xFFFF,-(%sp)		# save all registers
	clr.w	idleflg			# clear idle flag
	add.l	&1,sysinfo+V_INTR%	# count interrupts
	mov.l	%usp,%a0
	mov.l	%a0,60(%sp)		# save usr stack ptr
	mov.l	66(%sp),%a0		# fetch interrupt routine address
	#	movl	sp,sp@-	# doesn't work on 68020 (decrements first)
	mov.l	%sp,%a1			# save argument list pointer
	mov.l	%a1,-(%sp)		# push ap onto stack
	jsr	(%a0)			# jump to actual interrupt handler
	add.l	&4,%sp
	bsr	checksir%		# check for software interrupt requests
	btst	&5,70(%sp)		# did we come from user mode?
	bne	LS%10			# no, just continue
	mov.w	&0x2700, %sr		# Do streams scheduling (crit. section)
	tst.b	qrunflag		# Are there streams to do?
	beq	LQR%9
	tst.b	queueflag		# Are they already being done?
	bne	LQR%9
	mov.b	&1, queueflag		# OK. Do them
	mov.w	&0x2000, %sr		# Exit crit. section
	mov.l	stream_run, %a0		
	jsr	(%a0)			# Call streams scheduler
	clr.b	queueflag		# Tidy up on the way out
	bra	LQR%10
LQR%9:	mov.w	&0x2000, %sr
LQR%10:
	tst.l	runrun			# should we reschedule?
	beq	LS%10			# no, just return normally
	mov.l	&256,%d0		# 256 is reschedule trap number
	bra	resched%		# go back into trap

LS%10:	mov.l	60(%sp),%a0
	mov.l	%a0,%usp		# restore usr stack ptr
	movm.l	(%sp)+,&0x7FFF		# restore all other registers
	add.w	&10,%sp			# sp, pop fault pc, and alignment word
	rte				# return from whence called

	# General purpose code

	global	getusp,getsr

getusp:	mov.l	%usp,%a0		# get the user stack pointer
	mov.l	%a0,%d0
	rts	

getsr:	mov.l	&0,%d0			# get the sr
	mov.w	%sr,%d0
	rts	

#ifdef	mc68020
	global	ffs
ffs:
	tst.l	4(%sp)
	bne	ffs1%
	clr.l	%d0
	br	ffs2%
ffs1%:
	short	0xedef, 0x001f, 0x0004		# bfffo	4(%sp){&0:&31},%d0
	neg.l	%d0
	add.l	&32, %d0
ffs2%:
	mov.l	%d0, %a0
	rts
#endif	mc68020

	#	Software interrupt request handler
	data
	global sir
sir:
	long	0
	text

	global	siron
siron:
	mov.l	&1,sir
	rts

	global	softint, svstak, dnisr
checksir%:
	tst.l	sir			# soft interrupt pending?
	beq	LS%11			# no: return
	mov.w	&0x2700,%sr		# set priority 7
	tst.b	innet%			# already in net code ?
	bne	LS%11			# yes: return
	tst.b	dnisr			# delay net interrupt ?
	bne	LS%11			# yes: return
	mov.b	&1,innet%		# no: set flag -> in the net code
	mov.l	%sp,svstak		# get new stack
	mov.l	&netstak+2996,%sp
	jsr	softint			# do tasks
	mov.l	svstak,%sp
	clr.b	innet%			# clear flag
LS%11:
	rts	

	text	
	# MMB Primitives
	data	
	text	
	global	mmb_on
	# Turn on MMB (write to root pointer and translation control register)
mmb_on:
	link	%fp,&-4
#ifdef	PMMU
	lea     cpu_rp,%a0
	#       pmove   a0@,crp
	short   0xF010
	short   0x4C00
#ifdef	PMMUMMB
	#       pmove   &0x81A07780,tc  # 32-bit enabled
	short   0xF03C
	short   0x4000
	short   0x81A0
	short   0x7780
#else	PMMUMMB
	lea     sup_rp,%a0
	#       pmove   a0@,srp
	short   0xF010
	short   0x4800
	#       pmove   &0x82D0A900,tc  # load translation control register
	short   0xF03C
	short   0x4000
	short   0x82D0
	short   0xA900
#endif	PMMUMMB
	#       pflusha                 # flush all atc entries
	short   0xF000
	short   0x2400
#else	PMMU
	mov.l	&0x7,%d0		# setup system mode write
	#	movecl	d0,dfc
	short	0x4e7b
	short	0x0001
	mov.l	&0,%d0
	mov.l	&MMUADDR+4,%a0		# turn off TCR (cycle operation)
	#	movesl	d0,a0@
	short	0x0e90
	short	0x0800
	mov.l	&MMUADDR,%a0		# cycle root pointer
	#	movesl	d0,a0@
	short	0x0e90
	short	0x0800
	mov.l	rootbl,%d0
	mov.l	&MMUADDR,%a0		# set root pointer reg to rootbl
	#	movesl	d0,a0@
	short	0x0e90
	short	0x0800
	mov.l	&0x81A07780,%d0		# 32-bit enabled
	mov.l	&MMUADDR+4,%a0		# write to TCR - turn on mapping
	#	movesl	d0,a0@
	short	0x0e90
	short	0x0800
#endif	PMMU

	mov.l	&VECBASE+KSTART,%d0	# Reload interrupt vector base register
	short	0x4E7B			#	movec
	short	0x0801			#	d0,vbr
	mov.l   &1,mmu_on
	unlk	%fp
	rts	

	global	fl_atc, fl_sysatc, fl_usratc

fl_sysatc:
#ifndef MMB
	#       pflushs &4,&4           # flush all supervisor atc entries
	short   0xF000
	short   0x3494
	rts
#endif MMB

fl_usratc:
#ifndef MMB
	lea     cpu_rp,%a0
	#       pflushr (%a0)           # flush current user's atc entries
	short   0xF010
	short   0xA000
	rts
#endif MMB

fl_atc:
#ifdef	PMMU
	#       pflusha                 # flush all atc entries
	short   0xF000
	short   0x2400
#else
	mov.l	%d0,-(%sp)
	mov.l	%a0,-(%sp)
	mov.l	&0x7,%d0		# setup system mode write
	#	movecl	d0,dfc
	short	0x4e7b
	short	0x0001
	mov.l	rootbl,%d0
	mov.l	&MMUADDR,%a0		# write arg to root pointer
	#	movesl	d0,a0@
	short	0x0e90
	short	0x0800
	mov.l	(%sp)+,%a0
	mov.l	(%sp)+,%d0
#endif	PMMU
	rts	

	# Code to catch all interrupts and flush the 68020 cache
	# Needed due to improper storage of instruction stream during
	# 68020 instruction fetches
	global catchintr
catchintr:	
	sub.l	&4,%sp			# leave a hole
	mov.l	%d0,-(%sp)		# save registers that we need
	mov.w	%sr,-(%sp)		# save the current status register
	mov.w	&HIGH%,%sr		# spl7

	mov.l	m20cache,%d0		# set enable/disable bit for 68020 CACR
	or.l	&CACHECLR,%d0		# flush 68020 cache
	#	movecl	d0,cacr
	short	0x4e7b
	short	0x0002
	#	movecl	d0,cacr		# repeat required due to bug on 68020
	short	0x4e7b
	short	0x0002
	nop				# required due to bug on 68020
	#	movecl	d0,cacr		# repeat required due to bug on 68020
	short	0x4e7b
	short	0x0002
	nop

	mov.w	(%sp)+,%sr		# restore status register

	mov.w	14(%sp),%d0		# determine interrupt number
	and.l	&0x0FFF,%d0
	add.l	&ivect,%d0
	mov.l	%d0,4(%sp)		# store it in the usual bsr location
	mov.l	(%sp)+,%d0		# restore registers
	rts

	global	caller
	global	mchlast
caller:
mchlast:
	link	%fp,&0
	mov.l	(%fp),%a0
	mov.l	4(%a0),%d0
	mov.l	%d0,%a0
	unlk	%fp
	rts

	data	
innet%:	byte	0
dnisr:	byte	0
	comm	svstak,4
	comm	netstak,3000
