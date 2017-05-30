O			= .o
A			= .a
E 			=
CC 			= cc
AR			= ar ruv
CFLAGS 		= -g -Wall
BUILDDIR	= build
B 			= $(BUILDDIR)/
T 			= $(TSTDIR)/

all: c5

c5: $Bc5$E

RCCOBJS=$Bmain$O

$Bc5$E:	$Bmain$O $Blibrcc$A $(EXTRAOBJS)
		$(CC) $(CFLAGS) -o $@ $Bmain$O $Blibrcc$A $(EXTRAOBJS)

$Blibrcc$A:	$(RCCOBJS)
		$(AR) $@ $(RCCOBJS)

$Bmain$O:	src/main.c;	$(CC) $(CFLAGS) -c -Isrc -o $@ src/main.c

clean:
		$(RM) $B*$O
		$(RM) $Brcc$E
		$(RM) $B*.a