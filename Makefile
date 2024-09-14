LIBS  = -lm
CFLAGS = -Wall

SRCC=$(wildcard *.c)
SRCC=$(wildcard *.cc)

all: $(SRCC)
    gcc -o $@ $^ $(CFLAGS) $(LIBS)