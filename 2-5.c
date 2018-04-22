/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>

int main(void){
    int i,j;
    for (i=0;i < 10;i++){
        for(j=0;j<10-i;j++){
            printf("%d",i+j);
        }
        printf("\n");
    }
    return 0;
}