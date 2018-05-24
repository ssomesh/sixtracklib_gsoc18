# adding the floating point numbers in a list

# usage: python vectorSum_double.py <size of the vector>

import random
import sys
import timeit

#function to sum the elements in a vector
def vectorSum (arr, sz):
  sum = 0
  for i in range (0,sz):
    sum += arr[i]

  return sum

limit = 10
sz = int(sys.argv[1])
arr = [] 
for i in range (0,sz):
  arr.append(random.uniform(0,1) * limit)

#print (*arr)
#sum = vectorSum(arr, sz)
#print (sum)

setup = """
import random
import sys
import timeit

limit = 10
sz = int(sys.argv[1])
arr = [] 
for i in range (0,sz):
  arr.append(random.uniform(0,1) * limit)

def vectorSum (arr, sz):
  sum = 0
  for i in range (0,sz):
    sum += arr[i]

  return sum
"""
# timing the code vectorSum(..)
t = timeit.Timer(stmt="vectorSum(arr,sz)", setup=setup)
print (t.timeit(10)/10)
