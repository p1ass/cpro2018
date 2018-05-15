/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

/*
unsigned int 符号なしの整数型
signed int 符号ありの整数型
よって、unsigned intを使った場合はi = 0 - 1は-1ではなく unsinged int型の最大値である2949672959になり、
whileループを抜けることができない。
*/

#include <stdio.h>
int main() {
    signed int i=5;
    while(i>=0) {
      printf("i=%d\n", i);
      i=i-1;
    }
    return 0;
}