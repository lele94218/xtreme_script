
O			= .o
A			= .a
E 			=
CC 			= cc
AR			= ar ruv
CFLAGS 		= -g -Wall
BUILDDIR	= build
B 			= $(BUILDDIR)/
T 			= $(TSTDIR)/

all: rcc

rcc: $Brcc$E

RCCOBJS=$Balloc$O \
		$Bmain$O \
		$Bstring$O

$Brcc$E:	$Bmain$O $Blibrcc$A $(EXTRAOBJS)
		$(CC) $(CFLAGS) -o $@ $Bmain$O $Blibrcc$A $(EXTRAOBJS)

$Blibrcc$A:	$(RCCOBJS)
		$(AR) $@ $(RCCOBJS)

$Balloc$O:	src/alloc.c;	$(CC) $(CFLAGS) -c -Isrc -o $@ src/alloc.c
$Bmain$O:	src/main.c;	$(CC) $(CFLAGS) -c -Isrc -o $@ src/main.c
$Bstring$O:	src/string.c;	$(CC) $(CFLAGS) -c -Isrc -o $@ src/string.c

clean:
		$(RM) $B*$O
		$(RM) $Brcc$E
		$(RM) $B*.a