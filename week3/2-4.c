/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>

int main(void){
    int x;
    int sum = 0;

    for(x=0;x < 100;x++){
        if (x % 3 != 0){
          sum += x;
        }
    }
    printf("sum=%d\n", sum);
    return 0;
}