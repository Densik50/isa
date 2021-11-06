#Autor: Daniel Paulovic <xpaulo04>

CC=g++
CFLAGS=-Wall -Wextra -pedantic -std=c++11
PROJECT=myclient

all:	$(PROJECT)

$(PROJECT):	$(PROJECT).cpp
	$(CC) $(CFLAGS) $(PROJECT).cpp -o $(PROJECT)

clean:
	rm -f *.o

clean-all:
	rm -f *.o $(PROJECT) xpaulo04.tar

tar: clean
	tar -cvf xpaulo04.tar *