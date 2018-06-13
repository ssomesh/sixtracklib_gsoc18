#!/bin/bash

make
# printing the heading
printf "N \t Kernel time(ns) with Global memory \t Kernel time(ns) with Local memory\n" > results.txt
for n in  128 256 512 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576
do
#n=3225
./reduce_v2a $n > "temp1.txt"
./reduce_v2b $n > "temp2.txt"
# collecting the info
ans_with_local_memory=`tail -1 temp1.txt`
#echo $ans_with_local_memory

ans_with_global_memory=`tail -1 temp2.txt`
#echo $ans_with_global_memory

if [ ! "$ans_with_global_memory" = "$ans_with_local_memory" ]; then
  echo "Alert! Answers do not match."
  exit
fi


x=`awk -F" " '{print $(NF-1)}' temp1.txt | tail -3 | head -1` # sed -n '3p'`
y=`awk -F" " '{print $(NF-1)}' temp2.txt | tail -3 | head -1`  # sed -n '3p'`

printf "%d\t %e\t%e\n" "$n" "$y" "$x" >> results.txt
done

rm -f temp1.txt temp2.txt # deleting the unwanted files
