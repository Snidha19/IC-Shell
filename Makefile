CC = gcc

all: icsh

icsh: icsh.c
	$(CC) -o $@ $<

clean:
	rm -r icsh icsh.dSYM