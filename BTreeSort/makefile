CC=g++
CFLAGS=-c -g -fopenmp -std=c++17
LFLAGS=-g

ifeq ($(OS), Windows_NT)
    detectOS := win
else
    detectOS := $(shell uname -s)
endif

$(info Detected OS = $(detectOS))

ifeq ($(detectOS), win)
    LFLAGS += -lntdll
endif

msort: main.o timer.o
	$(CC) main.o timer.o $(LFLAGS) -o msort

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o
	rm -f msort
	rm -f msort.exe