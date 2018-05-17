from flatobject import Buffer,Object, f64, u64, i64, Field

data=Buffer()

class A(Object):
    fa = f64()
    fb = u64()

assert A.fa.index==0
assert A.fa.name=='fa'
assert A.fb.index==1
assert A.fb.name=='fb'


a=A(data,fa=1.1,fb=2)

assert a.fa==1.1
assert a.fb==2


class B(Object):
    fa = f64(size=10)
    fb = u64(size=5)

b=B(data,fa=1.2,fb=3)

assert a.fa==1.1
assert a.fb==2
assert b.fa[0]==1.2
assert b.fb[-2]==3


class C(Object):
    nn = u64(const=True)
    fb = u64(size='nn')
    vv = u64(value=2)

c=C(data,nn=10,fb=3)

assert len(c.fb)==c.nn
assert c.vv==2

