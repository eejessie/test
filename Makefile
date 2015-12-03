# This is a c++ code. Using g++ compiler.
CC=g++

# Compiler flags.
CFLAGS= -g 

VPATH = /home/eejessie/work/AC/als_sat/class: /home/eejessie/work/AC/als_sat/lib: /home/eejessie/work/AC/als_sat/cudd:  /home/eejessie/work/CUDD/cudd-2.4.1/cudd: /home/eejessie/work/AC/abc:


WHERE	= ../../CUDD/cudd-2.4.1
WHERE_ABC	= ../abc
INCLUDE = $(WHERE)/include
LIBS	= $(WHERE)/dddmp/libdddmp.a $(WHERE)/cudd/libcudd.a \
	$(WHERE)/mtr/libmtr.a $(WHERE)/st/libst.a $(WHERE)/util/libutil.a \
	$(WHERE)/epd/libepd.a $(WHERE_ABC)/libabc.a


MAINSRC = main.cc 
OTHSRC1 = CircuitNode.cc CircuitLine.cc FaultList.cc TestList.cc radix_convert.cc file_operations.cc forward_implication.cc string_convert.cc HashTable.cc queue.cc

OTHSRC2 =  read_circuit_v2.cc adjust_ln.cc copy_point_vector.cc print_circuit.cc merge_circuit.cc atpg.cc  red_forward_implication_v3.cc red_forward_deletion_v2.cc red_backward_deletion.cc  helper.cc ATPG_checker.cc parallel_test_simulation_bdd.cc run_logic_simulation.cc 

OTHSRC3 = rem_red_sat.cc opt_rar_sat.cc write_lisp.cc br_lisp.cc write_bench.cc br_bench.cc call_abc.cc

OTHSRC4 =  compute_MA.cc recursive_learning_new.cc implicate_new.cc upd_circuit_plus.cc idtf_unt.cc recursive_learning_un.cc implicate_un.cc upd_circuit.cc sim_br.cc 

OTHSRC5 =  cudd_build.cc cudd_comp.cc  cudd_dst.cc bnet.cc ntr.cc

#OTHSRC6 = top_info.cc mback.cc 


SRC = $(MAINSRC) $(OTHSRC1) $(OTHSRC2) $(OTHSRC3) $(OTHSRC4) $(OTHSRC5) 
OBJ = $(SRC:.cc=.o)
TARGET = main_cudd

# make all runs.
all: $(TARGET)


%.o: %.cc
	$(CC) $(CFLAGS) -o $@  -c $< 

	

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)  $(LIBS) -lm -ldl -rdynamic -ltermcap -lpthread -lrt 


# make clean
clean:
	rm -f $(OBJ) $(TARGET)

