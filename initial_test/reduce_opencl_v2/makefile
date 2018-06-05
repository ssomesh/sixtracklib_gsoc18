all: reduce_v2a reduce_v2b 

reduce_v2a: reduce_v2a.cpp
	g++ -std=c++11 -o reduce_v2a reduce_v2a.cpp -lOpenCL -I/usr/local/cuda-8.0/include/ -L/usr/lib64/nvidia/

reduce_v2b: reduce_v2b.cpp 
	g++ -std=c++11 -o reduce_v2b reduce_v2b.cpp -lOpenCL -I/usr/local/cuda-8.0/include/ -L/usr/lib64/nvidia/

clean:
	rm -f reduce_v2a reduce_v2b

