CC = g++
CFLAGS = -Wall
DEBUG = -g -D DEBUG
TARGET = main.o
SRCS = main.cpp
MAIN = lwt

all: $(MAIN)
$(MAIN): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)
debug: $(SRCS)
	$(CC) $(DEBUG) $(SRCS) -o $(TARGET)
profile: $(SRCS)
	$(CC) $(PROFILE) $(SRCS) -o $(TARGET)
	./$(TARGET)
	gprof $(TARGET) gmon.out | ./gprof2dot.py | dot -Tpng -o output.png
clean:
	$(RM) *.o $(TARGET)
