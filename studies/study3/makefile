all: vectorisable_example vectorisable_example_vect makefile
	./vectorisable_example
	./vectorisable_example_vect


vectorisable_example: vectorisable_example.cpp makefile
	g++ $< -o $@ -O2

vectorisable_example_vect: vectorisable_example.cpp makefile
	g++ $< -o $@ -O2 -ftree-vectorize -march=native -fopt-info-all=$@.info
