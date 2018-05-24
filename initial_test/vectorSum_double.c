// Summing up floating point numbers in a 1-D array


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>

// method for populating the vector in the range [0,LIMIT)
void fillArray(double* arr, uint64_t N) {
  const uint64_t LIMIT = 10;
  unsigned i;
  double x;
  srand48(time(NULL));
  for(i=0; i<N; i++)
        arr[i] = drand48() * LIMIT ;
}
 
 // display the vector
printArray(double * arr, uint64_t N) {
  uint64_t i;
  for(i=0; i<N; i++)
  {
    printf("%4.4g",arr[i]);
    printf(((i%8) != 7) ? "\t" : "\n"); // printing 8 numbers on a line
  }
  printf("\n");
}

double rtclock()
{
  struct timezone Tzp;
  struct timeval Tp;
  uint64_t stat;
  stat = gettimeofday (&Tp, &Tzp);
  if (stat != 0) printf("Error return from gettimeofday: %d",stat);
  return(Tp.tv_sec + Tp.tv_usec*1.0e-6);
}

double vectorSum_v0(double * arr, uint64_t N) {
  unsigned i;
  double sum = 0.0f;
  for(i=0; i<N; i++)
    sum += arr[i];

  return sum;
}

void vectorSum_v1(double * arr, uint64_t N, double * sum) {
  unsigned i;
  *sum = 0.0f;
  for(i=0; i<N; i++)
    *sum += arr[i];
}

int main(int argc, char** argv) {
  if(argc != 2) {
    printf("Usage: %s <vector_size>\n", argv[0]);
    exit(1);
  }
  uint64_t sz = atoi(argv[1]);
  double * vec;
  vec = (double*) malloc(sizeof(double) * sz);
  fillArray(vec, sz);
  //printArray(vec, sz);

  double clkbegin, clkend;
  double t;
  clkbegin = rtclock();
  // the function for adding the numbers in a vector
  double sum = vectorSum_v0(vec, sz);
  clkend = rtclock();
  t = clkend-clkbegin;
  printf("The sum of vector from version0 = %4.4g\n", sum);
  printf("Stats for version0: %.1f GFLOPS; Time = %.3f sec;\n", (1.0e-9*sz)/t,t);

  double sum1;
  clkbegin = rtclock();
  vectorSum_v1(vec, sz, &sum1);
  clkend = rtclock();
  t = clkend-clkbegin;
  printf("The sum of vector from version1 = %4.4g\n", sum1);
  printf("Stats for version1: %.1f GFLOPS; Time = %.3f sec;\n ", (1.0e-9*sz)/t,t);
  



  return 0;
}
