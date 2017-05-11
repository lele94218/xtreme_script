
O			=.o
E 			=
CC 			= cc
CFLAGS 		= -g
BUILDDIR	= build
B 			= $(BUILDDIR)/
T 			= $(TSTDIR)/

all: rcc

rcc: $Brcc$E

RCCOBJS=$Balloc$O \
		$Bmain$O

$Brcc$E:	$Bmain$O $(EXTRAOBJS)
		$(CC) $(CFLAGS) -o $@ $Bmain$O $(EXTRAOBJS)

$Blibrcc$A:	$(RCCOBJS)
		$(AR) $@ $(RCCOBJS); $(RANLIB) $@ || true

$Balloc$O:	src/alloc.c;	$(CC) $(CFLAGS) -c -Isrc -o $@ src/alloc.c
$Bmain$O:	src/main.c;	$(CC) $(CFLAGS) -c -Isrc -o $@ src/main.c