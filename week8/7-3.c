/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>

void inc(int *a){
    *a = *a+ 1;
}
int main(void){
    int n = 0;
    printf("input a digit: ");
    scanf("%d",&n);
    inc(&n);
    printf("output : %d\n",n);
    return 0;
}
