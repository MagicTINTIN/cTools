# Compiler settings
CC = gcc
CXX = g++

# Compiler flags
CFLAGS = -Wall -g
CXXFLAGS = -Wall -g

# Default target
all: client server move

client: wclient.c webcam.c
	gcc -o wclient.o wclient.c webcam.c

move: wmove.c wmove.c
	gcc -o wmove.o wmove.c

server: wserver.c
	gcc -o wserver.o wserver.c

# Clean the object files and executables
clean:
	rm -f *.o