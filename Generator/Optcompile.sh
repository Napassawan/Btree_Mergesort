#g++ -$1 -o mainBQ parBQ.cpp -fopenmp -m64 -march=native
#g++ -$1 -o par par.cpp -m64
clang++ -Xclang -fopenmp -std=c++11 -Wall -o parBQ $1 -O2 -lomp
