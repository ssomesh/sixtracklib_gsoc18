all: reduce_v2a reduce_v2b reduce_v2a_1 reduce_v2b_1

reduce_v2a: reduce_v2a.cpp
	g++ -std=c++11 -o reduce_v2a reduce_v2a.cpp -lOpenCL -I/usr/local/cuda-8.0/include/ -L/usr/lib64/nvidia/
	./reduce_v2a 4096

reduce_v2a_1: reduce_v2a_1.cpp
	g++ -std=c++11 -o reduce_v2a_1 reduce_v2a_1.cpp -lOpenCL -I/usr/local/cuda-8.0/include/ -L/usr/lib64/nvidia/
	./reduce_v2a_1 4096

reduce_v2b: reduce_v2b.cpp 
	g++ -std=c++11 -o reduce_v2b reduce_v2b.cpp -lOpenCL -I/usr/local/cuda-8.0/include/ -L/usr/lib64/nvidia/
	./reduce_v2b 4096

reduce_v2b_1: reduce_v2b_1.cpp
	g++ -std=c++11 -o reduce_v2b_1 reduce_v2b_1.cpp -lOpenCL -I/usr/local/cuda-8.0/include/ -L/usr/lib64/nvidia/
	./reduce_v2b_1 4096

clean:
	rm -f reduce_v2a reduce_v2b reduce_v2b_1 reduce_v2a_1

