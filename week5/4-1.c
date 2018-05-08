/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>

int main(void){
    int x[5] = {4, 1, 8,2, 9};
    int i;
    int range;

    for(i=0;i<5;i++){
        printf("x[%d] = %d\n",i,x[i]);
    }

    //先に初期化しておく。0で初期化するとxが負の要素のみだった場合にバグる
    range = x[0];

    for(i=0;i<5;i++){
        if(x[i] > range){
            range = x[i];
        }
    }

    printf("max=%d\n",range);
    return 0;
}