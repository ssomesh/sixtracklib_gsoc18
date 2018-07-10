#! /bin/bash

echo "beam elements = 1000"
echo "number of turns = 100"
  echo "--------""--------""--------""--------"
  echo "--------""--------""--------""--------"
for particles in 1000 2500 5000 10000 25000 50000
do
  echo "Particles = " $particles
  echo ""
  ./build/drift_cpu $particles
  echo "--------""--------""--------""--------"
done
