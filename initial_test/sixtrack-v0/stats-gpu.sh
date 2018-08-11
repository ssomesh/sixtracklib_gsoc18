#! /bin/bash

#make
echo "beam elements = 1000"
  echo "--------""--------""--------""--------"
  echo "--------""--------""--------""--------"
for particles in 1000 2500 5000 10000 25000 50000 100000 #250000 500000 1000000 2500000 5000000 10000000 25000000
do
  echo "number of turns = 50"
  echo "Particles = " $particles
  echo ""
#  ./build/parallel_beam_elements $particles 0 # on the AMD GPU
  ./build/parallel_beam_elements_allinsequence $particles 50 1 1 # on the nvidia GPU
#  ./build/parallel_beam_elements-finalversion $particles 50 4 # on the intel xeon cpu (techlab machine)
#  ./build/parallel_beam_elements $particles 2 # on the cpu 
  echo "--------""--------""--------""--------"
done
for particles in 100000 250000 500000 1000000 2500000 5000000 10000000 25000000  # note 100,000 appears in both the for loops
do
  echo "number of turns = 5"
  echo "Particles = " $particles
  echo ""
#  ./build/parallel_beam_elements $particles 0 # on the AMD GPU
  ./build/parallel_beam_elements_allinsequence $particles 5 1 1 # on the nvidia GPU
#  ./build/parallel_beam_elements-finalversion $particles 5 4 # on the intel xeon cpu (techlab machine)
#  ./build/parallel_beam_elements $particles 2 # on the cpu 
  echo "--------""--------""--------""--------"
done
