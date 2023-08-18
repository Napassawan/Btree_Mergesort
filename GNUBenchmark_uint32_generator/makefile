CC=g++
CFLAGS=-c -g -fopenmp
CFLAGS_AFTER=-g -fopenmp
SOURCE=main.cpp main_LAL.cpp

OBJ=$(SOURCE:.cpp=.o)
EXECUTABLE=main
HEADER=lib/utilizecpu.h lib/ioutility.h lib/cmpfunc.h
OBJHEAD=$(HEADER:.h=.o)

generator_new: generator_new.cpp
	$(CC) -o generator_new generator_new.cpp $(CFLAGS_AFTER)

mainBQ: parBQ.cpp
	$(CC) -o mainBQ parBQ.cpp $(CFLAGS_AFTER)
mainMW: parMW.cpp
	$(CC) -o mainMW parMW.cpp $(CFLAGS_AFTER)

%.o : %.cpp
	$(CC) $(CFLAGS) -o $@ $<

all: 
	make clean
	make mainBQ
	make mainMW

##########################
# Clean
##########################
clean :
	rm -f *.o
	rm -f lib/*.o
	rm -f main*
