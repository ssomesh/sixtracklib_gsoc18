# adding the integers in a list

# usage: python vectorSum_int.py <size of the vector>

from random import randint
import sys
import timeit
#import random

#function to sum the elements in a vector
def vectorSum (arr, sz):
  sum = 0
  for i in range (0,sz):
    sum += arr[i]

  return sum


sz = int(sys.argv[1])
arr = [] 
for i in range (0,sz):
  arr.append(randint(0,10))

#arr.append(random.uniform(0, 1))
#print (*arr)
#sum = vectorSum(arr, sz)
#print (sum)

setup = """
from random import randint
import sys
import timeit
sz = int(sys.argv[1])
arr = [] 
for i in range (0,sz):
  arr.append(randint(0,10))

def vectorSum (arr, sz):
  sum = 0
  for i in range (0,sz):
    sum += arr[i]

  return sum
"""
# timing the code vectorSum(..)
t = timeit.Timer(stmt="vectorSum(arr,sz)", setup=setup)
print (t.timeit(10)/10)
