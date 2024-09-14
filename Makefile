# Compiler settings
CC = gcc
CXX = g++

# Compiler flags
CFLAGS = -Wall -g
CXXFLAGS = -Wall -g

# Find all .c and .cpp/.cc files
C_SRC := $(wildcard *.c)
CXX_SRC := $(wildcard *.cpp *.cc)

# Create object files and executables names
C_OBJ := $(C_SRC:.c=.o)
CXX_OBJ := $(CXX_SRC:.cpp=.o)
CXX_OBJ := $(CXX_OBJ:.cc=.o)

# Default target
all: $(C_OBJ) $(CXX_OBJ)

# Rule to compile .c files into executables
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Rule to compile .cpp/.cc files into executables
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) $< -o $@

# Clean the object files and executables
clean:
	rm -f *.o