all: ex1 ex1_vect makefile
	./ex1
	./ex1_vect


ex1: ex1.cpp makefile
	g++ $< -o $@ -O2

ex1_vect: ex1.cpp makefile
	#g++ $< -o $@ -O2 -ftree-vectorize -march=native -fopt-info-all=$@.info
	g++ $< -o $@ -O2 -ftree-vectorize -march=native -fopt-info
