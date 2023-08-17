CC=g++
CFLAGS=-c -g -fopenmp #-m64 -march=native
CFLAGS_AFTER= -O2 -fopenmp #-m64 -march=native
SOURCE=main.cpp main_LAL.cpp

OBJ=$(SOURCE:.cpp=.o)
EXECUTABLE=main
HEADER=lib/utilizecpu.h lib/ioutility.h lib/cmpfunc.h
OBJHEAD=$(HEADER:.h=.o)

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
