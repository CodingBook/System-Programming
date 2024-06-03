#include <stdio.h>

int isGreater(int x, int y) {

     int r = y + (~x+1);

  r = (r>>31)&0x1;
    x = (x>>31)&0x1;
  y = (y>>31)&0x1;


  int check = (y^x)&(y^r);

   return r^check;
}

int allEvenBits(int x) {

   int a = 0x5;

    a &= x;
    a &= x>>4;
    a &= x>>8;
    a &= x>>12;
    a &= x>>16;
    a &= x>>20;
    a &= x>>24;
    a &= x>>28;

    printf("\n%d", a==0x5);

   return 0;
}

int main(){

    isGreater(0x80000000, 0x7fffffff);
    isGreater(0x80000000, 0x80000000);
    isGreater(0x7fffffff, 0x7fffffff);

    allEvenBits(0x7ffffffe);
    allEvenBits(0x80000000);
    allEvenBits(0x55555555);

    return 0;
}

