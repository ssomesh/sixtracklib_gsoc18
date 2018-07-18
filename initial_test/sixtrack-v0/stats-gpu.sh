#! /bin/bash

#make
echo "beam elements = 1000"
echo "number of turns = 100"
  echo "--------""--------""--------""--------"
  echo "--------""--------""--------""--------"
for particles in 1000 2500 5000 10000 25000 50000 100000 250000 500000 1000000 2500000 5000000 10000000
do
  echo "Particles = " $particles
  echo ""
  ./build/parallel_drift_cleanedup $particles
  echo "--------""--------""--------""--------"
done
