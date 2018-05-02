/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/
#include <stdio.h>
 int main(void){
    int x;
    printf("Type an integer: \n");
    scanf("%d", &x);

    if (x % 3 == 0){
         printf("YES\n");
    }else if(x % 5 == 0) {
        printf("YES\n");
    }else{
        printf("NO\n");
    }
    return 0;
 }