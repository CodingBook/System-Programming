CC = gcc
CFLAGS = -W -Wall
TARGET = diary.out

all : $(TARGET)

$(TARGET) : main.o add.o sub.o mul.o div.o memo.o calendar.o
	$(CC) $(CFLAGS) -o $@ $^

main.o : main.c
	$(CC) $(CFLAGS) -c -o $@ $^

add.o : add.c
	$(CC) $(CFLAGS) -c -o $@ $^

sub.o : sub.c
	$(CC) $(CFLAGS) -c -o $@ $^

mul.o : mul.c
	$(CC) $(CFLAGS) -c -o $@ $^

div.o : div.c 
	$(CC) $(CFLAGS) -c -o $@ $^

memo.o : memo.c
	$(CC) $(CFLAGS) -c -o $@ $^

calendar.o : calendar.c
	$(CC) $(CFLAGS) -c -o $@ $^


clean : 
	rm -rf *.o
	rm -rf *.out


