#! /bin/bash

#make
echo "beam elements = 1000"
echo "number of turns = 100"
  echo "--------""--------""--------""--------"
  echo "--------""--------""--------""--------"
for particles in 1000 2500 5000 10000 25000 50000 100000 250000 500000 1000000 2500000 5000000 10000000 25000000
do
  echo "Particles = " $particles
  echo ""
#  ./build/parallel_beam_elements $particles 0 # on the AMD GPU
  ./build/parallel_beam_elements $particles 1 # on the nvidia GPU
#  ./build/parallel_beam_elements $particles 2 # on the cpu 
  echo "--------""--------""--------""--------"
done
