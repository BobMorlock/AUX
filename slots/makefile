# Local File Group Macros

HDRS =		.
HFILES =	slots.h
SCFILES =	slottest.c etheraddr.c

LIBCFILES =	slot_library.c slot_PRAM.c slot_brd_ven.c \
		slot_brd_flg.c slot_brd_id.c slot_brd_nam.c slot_brd_typ.c \
		slot_byte.c slot_data.c slot_dir.c slot_eth_add.c \
		slot_long.c slot_prim.c slot_prt_num.c slot_rev_lvl.c \
		slot_rsrc.c slot_rsrcl.c slot_ser_num.c slot_struct.c \
		slot_ven_id.c slot_word.c

SOFILES =	slottest.o

LINTCFILES =	$(LIBCFILES)

LIBOFILES =	slot_library.o slot_PRAM.o slot_brd_ven.o \
		slot_brd_flg.o slot_brd_id.o slot_brd_nam.o slot_brd_typ.o \
		slot_byte.o slot_data.o slot_dir.o slot_eth_add.o \
		slot_long.o slot_prim.o slot_prt_num.o slot_rev_lvl.o \
		slot_rsrc.o slot_rsrcl.o slot_ser_num.o slot_struct.o \
		slot_ven_id.o slot_word.o

ALLCFILES = 	$(LIBCFILES) $(SCFILES)
ALLOFILES = 	$(LIBOFILES) $(SOFILES)
ALLFILES = 	$(HFILES) $(ALLCFILES)

LIB =		slots.a

# Make rules
SHELL = /bin/sh
# Defines:
#	IKKI for IKKI
#	-DDEBUG
CC =	cc
FLAGS =		-DIKKI -I$(HDRS) -DTEST
CFLAGS =	-c $(FLAGS)

lib:	$(LIBOFILES)
		rm -f $(LIB)
		ar cr $(LIB) $(LIBOFILES)


slottest:	slottest.o $(LIB)
		$(CC) -o slottest slottest.o slots.a

etheraddr:	etheraddr.o $(LIB)
		$(CC) -o etheraddr etheraddr.o slots.a

.c.o:
		$(CC) $(CFLAGS) $*.c

clean	:
		rm -f $(ALLOFILES) $(LIB)

.PRECIOUS:	lnt save

lnt:		$(LINTCFILES)
		    lint $(FLAGS) -u -DLINT $(LINTCFILES) | tee lnt"
#		    sh -ic "lint -abh $(LINTCFILES) -ltermcap | tee lnt"

printall:
		@rm -f print
		@make print

print:		$(ALLFILES) makefile
		enscript -2r $?; touch print

names:		$(ALLFILES)
		    echo "$(ALLFILES)" > names

tags:		$(ALLCFILES)
			ctags $(ALLCFILES)

depend:		$(ALLCFILES)
			echo '#start of automatic dependencies' > X1
			grep '^#include' *.c /dev/null | grep '"' > X2
			sed -e 's/#include "/$(HDRS)\//' \
				-e 's/"//' \
				-e 's/\.c/.o/' \
				-e 's/\/\*.*\*\///' \
				<X2 >X3
			cat X3 >> X1
			cp makefile newmakefile
			(echo '/^#start of auto/,$$d'; echo '$$r X1'; \
				echo w; echo q) | /bin/ed - newmakefile
			@mv makefile oldmakefile
			@mv newmakefile makefile
			@rm X1 X2 X3 oldmakefile
			touch depend
			@echo DONE with depend

# *** Do not add rules below this point! ***
#(The following line is keyed on by the above rule.)
#start of automatic dependencies
slot_PRAM.o:./slots.h
slot_brd_ven.o:./slots.h
slot_brd_flg.o:./slots.h
slot_brd_id.o:./slots.h
slot_brd_nam.o:./slots.h
slot_brd_typ.o:./slots.h
slot_byte.o:./slots.h
slot_data.o:./slots.h
slot_dir.o:./slots.h
slot_eth_add.o:./slots.h
slot_library.o:./slots.h
slot_long.o:./slots.h
slot_prim.o:./slots.h
slot_prt_num.o:./slots.h
slot_rev_lvl.o:./slots.h
slot_rsrc.o:./slots.h
slot_rsrcl.o:./slots.h
slot_ser_num.o:./slots.h
slot_struct.o:./slots.h
slot_ven_id.o:./slots.h
slot_word.o:./slots.h
slottest.o:./slots.h
