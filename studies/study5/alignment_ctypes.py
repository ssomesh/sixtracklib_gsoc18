import ctypes as ct

fields=[
        ('a', ct.c_char),
        ('b', ct.c_int),
        ('c', ct.c_char)]

class tt(ct.Structure):
    _fields_=fields

print("standard packing")
for nn,c in tt._fields_:
  print(f"offsetof({nn})={getattr(tt,nn).offset}")
print()

class tt(ct.Structure):
    _pack_ =1
    _fields_=fields

print("pack=1")
for nn,c in tt._fields_:
  print(f"offsetof({nn})={getattr(tt,nn).offset}")
print()

class tt(ct.Structure):
    _pack_ =2
    _fields_=fields

print("pack=2")
for nn,c in tt._fields_:
  print(f"offsetof({nn})={getattr(tt,nn).offset}")
print()

class tt(ct.Structure):
    _pack_ =8
    _fields_=fields

print("pack=8 does not work")
for nn,c in tt._fields_:
  print(f"offsetof({nn})={getattr(tt,nn).offset}")
print()

