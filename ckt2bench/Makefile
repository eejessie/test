# This is a c++ code. Using g++ compiler.
CC=g++

# Compiler flags.
CFLAGS= -g 

VPATH = /home/eejessie/work/AC/als_sat/class: /home/eejessie/work/AC/als_sat/lib: 


MAINSRC = ckt2bench.cc 
OTHSRC1 = CircuitNode.cc CircuitLine.cc FaultList.cc TestList.cc radix_convert.cc file_operations.cc forward_implication.cc string_convert.cc 

OTHSRC2 =  read_circuit_v2.cc print_circuit.cc write_bench.cc helper.cc


SRC = $(MAINSRC) $(OTHSRC1)
OBJ = $(SRC:.cc=.o)
TARGET = ckt2bench

# make all runs.
all: $(TARGET)


%.o: %.cc
	$(CC) $(CFLAGS) -o $@  -c $< 

	

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)


# make clean
clean:
	rm -f $(OBJ) $(TARGET)

