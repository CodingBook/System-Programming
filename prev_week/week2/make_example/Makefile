CC = gcc
CFLAGS = -W -Wall
TARGET = diary

OBJECTS = memo.o calender.o main.o

.SUFFIXES : .o .c

%.o : %.c
	$(CC) $(CFLAGS) -c $^

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean : 
	rm -rf *.o
	rm -rf *.out


