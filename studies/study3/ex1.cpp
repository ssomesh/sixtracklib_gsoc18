#include <time.h>
#include <stdio.h>


int main(){
const unsigned int ArraySize = 2<<23;
float* a = new float[ArraySize];
float* b = new float[ArraySize];
float* c = new float[ArraySize];

float startTime = (float)clock()/CLOCKS_PER_SEC;

for (unsigned int j = 0; j< 200 ; j++) // some repetitions
   for ( unsigned int i = 0; i < ArraySize; ++ i)
      c[i] = a[i] * b[i];


float endTime = (float)clock()/CLOCKS_PER_SEC -startTime;

printf("%g \n", endTime);
}

