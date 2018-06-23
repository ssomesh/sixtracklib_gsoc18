#include "stdio.h"
#include "stdlib.h"
#include "stddef.h"

#define PR(ex,fmt) printf(#ex " : " #fmt "\n",ex);

#define MY_ALIGN(n) __attribute__((aligned(n)))

typedef struct MY_ALIGN(512) {
    int a;
    int b;
    int c;
} TestType ;

typedef struct {
    int a MY_ALIGN(128);
    int b MY_ALIGN(128);
    int c MY_ALIGN(128);
} TestType2;

void test(TestType2* infun){
    PR(((char*) &(infun->b)) - ((char*)&(infun->a)),%zd);
    PR(((char*) &(infun->c)) - ((char*)&(infun->a)),%zd);
    PR(infun->a,%d);
    PR(infun->b,%d);
    PR(infun->c,%d);
};

int main(){
    TestType o1;
    PR(sizeof(TestType),%zd);
    PR(offsetof(TestType,a),%zd);
    PR(offsetof(TestType,b),%zd);
    PR(offsetof(TestType,c),%zd);

    PR( (size_t) &o1 % 512,%zd);
    PR( (size_t) &o1 % 2048,%zd);

    PR(sizeof(TestType2),%zd);
    PR(offsetof(TestType2,a),%zd);
    PR(offsetof(TestType2,b),%zd);
    PR(offsetof(TestType2,c),%zd);

    TestType2 o2;
    o2.a=1;
    o2.b=2;
    o2.c=3;

    PR( *(int*)((char*)&(o2)+0),%d);
    PR( *(int*)((char*)&(o2)+128),%d);
    PR( *(int*)((char*)&(o2)+256),%d);
    *(int*)((char*)&(o2)+4)=23;
    PR(o2.b,%zd);
    PR(((char*) &(o2.b)) - ((char*)&(o2.a)),%zd);
    PR(((char*) &(o2.c)) - ((char*)&(o2.a)),%zd);

    TestType2 *o3 = (TestType2 *) aligned_alloc(8,sizeof(TestType2));
    o3->a=5;
    o3->b=6;
    o3->c=7;
    test(o3);
    test(&o2);
    return 0;
}

