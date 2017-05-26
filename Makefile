CC = gcc
CFLAGS = -Wall -g
FILES = ./cache_sim

all: ./cache_sim

clean:
	rm -rf $(FILES) *.o *~ 
