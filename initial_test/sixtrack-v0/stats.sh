#! /bin/bash

for particles in 1000 2500 5000 10000 25000 50000
do
  echo "--------""--------""--------""--------"
  echo "Particles = " $particles
  echo "--------""--------""--------""--------"
  ./build/drift_cpu $particles
done
