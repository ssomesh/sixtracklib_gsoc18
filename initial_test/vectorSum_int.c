// Summing up integers in a 1-D array

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

// method for populating the vector in the range [0,LIMIT)
void fillArray(int* arr, int N) {
  const int LIMIT = 10;
  int i;
  float x;
  srand48(time(NULL));
  for(i=0; i<N; i++)
        arr[i] = (int)(drand48() * LIMIT) ;
}
 
 // display the vector
printArray(int * arr, int N) {
  int i;
  for(i=0; i<N; i++)
  {
    printf("%d",arr[i]);
    printf(((i%8) != 7) ? "\t" : "\n"); // printing 8 numbers on a line
  }
  printf("\n");
}

double rtclock()
{
  struct timezone Tzp;
  struct timeval Tp;
  int stat;
  stat = gettimeofday (&Tp, &Tzp);
  if (stat != 0) printf("Error return from gettimeofday: %d",stat);
  return(Tp.tv_sec + Tp.tv_usec*1.0e-6);
}

int vectorSum_v0(int * arr, int N) {
  unsigned i;
  unsigned sum = 0;
  for(i=0; i<N; i++)
    sum += arr[i];

  return sum;
}

void vectorSum_v1(int * arr, int N, int * sum) {
  unsigned i;
  *sum = 0;
  for(i=0; i<N; i++)
    *sum += arr[i];
}

int main(int argc, char** argv) {
  if(argc != 2) {
    printf("Usage: %s <vector_size>\n", argv[0]);
    exit(1);
  }
  int sz = atoi(argv[1]);
  int * vec;
  vec = (int*) malloc(sizeof(int) * sz);
  fillArray(vec, sz);
  //printArray(vec, sz);

  double clkbegin, clkend;
  double t;
  clkbegin = rtclock();
  // the function for adding the numbers in a vector
  int sum = vectorSum_v0(vec, sz);
  clkend = rtclock();
  t = clkend-clkbegin;
  printf("The sum of vector from version0 = %d\n", sum);
  printf("Time for version0 = %.3f sec.\n",t); 

  int sum1;
  clkbegin = rtclock();
  vectorSum_v1(vec, sz, &sum1);
  clkend = rtclock();
  t = clkend-clkbegin;
  printf("The sum of vector from version1 = %d\n", sum1);
  printf("Time for version1 = %.3f sec.\n",t); 
  


  return 0;
}

// An observation: time for version0 is consistently greater than time for version1. 
// This might be happening because the time computation in version0 also includes the time for writing the final result returned by the function vectorSum_v0.
