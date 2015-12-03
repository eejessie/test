# This is a c++ code. Using g++ compiler.
CC=g++

# Compiler flags.
CFLAGS= -g


MAINSRC = dtob.cc
OTHSRC = 

SRC = $(MAINSRC) $(OTHSRC)
OBJ = $(SRC:.cc=.o)
#TARGET = $(SRC:.cc=)
TARGET = dtob

# make all runs.
all: $(TARGET)

%.o: %.cc
	$(CC) $(CFLAGS) -o $@ -c $<  

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) 

# make clean
clean:
	rm -f $(OBJ) $(TARGET)

