all: reducev1 reducev1a 

reducev1: reduce_v1.cpp
	g++ -std=c++11 -o reduce_v1 reduce_v1.cpp -lOpenCL -I/usr/local/cuda-8.0/include/ -L/usr/lib64/nvidia/
	./reduce_v1

reducev1a: reduce_v1a.cpp 
	g++ -std=c++11 -o reduce_v1a reduce_v1a.cpp -lOpenCL -I/usr/local/cuda-8.0/include/ -L/usr/lib64/nvidia/
	./reduce_v1a

clean:
	rm -f reduce_v1 reduce_v1a

