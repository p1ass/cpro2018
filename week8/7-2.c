/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/
#include <stdio.h>

int main(void){
    char str[] = "Sun";
    int n[] = {10,20,40};
    double m[]= {1.5,3.5,7.5};

    printf("value: %c, address: %p\n",str[0],&str[0]);
    printf("value: %c, address: %p\n",str[1],&str[1]);
    printf("value: %c, address: %p\n",str[2],&str[2]);

    printf("value: %d, address: %p\n",n[0],&n[0]);
    printf("value: %d, address: %p\n",n[1],&n[1]);
    printf("value: %d, address: %p\n",n[2],&n[2]);

    printf("value: %f, address: %p\n",m[0],&m[0]);
    printf("value: %f, address: %p\n",m[1],&m[1]);
    printf("value: %f, address: %p\n",m[2],&m[2]);


    return 0;
}
