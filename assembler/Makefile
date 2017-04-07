CC = gcc
CFLAGS = -Wall
DEBUG = -g -D DEBUG
TARGET = main.o
SRCS = main.c
MAIN = vm

all: $(MAIN)
$(MAIN): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)
debug: $(SRCS)
	$(CC) $(DEBUG) $(SRCS) -o $(TARGET)
clean:
	$(RM) *.o $(TARGET)
