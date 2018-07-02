/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/
#include <stdio.h>
#include <stdlib.h>
void rand_init(int n, float *o){
    int i;

    for (i = 0; i < n; i++){
        o[i] = ((float)rand() / ((float)RAND_MAX + 1)) * 2 - 1 ;
    }
}

void print(int m, int n, const float *x){
    int i, j;

    for (i = 0; i < m; i++){
        for (j = 0; j < n; j++){
            printf("%.4f ", x[i * n + j]);
        }
        printf("\n");
    }
}

void swap(float *pa, float *pb){
    float temp = *pa;
    *pa = *pb;
    *pb = temp;
}

void sort(int n,float * x){
    int i,j;
    for(i=0;i<n;i++){
        for(j=0;j<n-i-1;j++){
            if(x[j]>x[j+1]){
                swap(&x[j],&x[j+1]);
            }
        }
    }
}


int main(int argc, char *argv[]) {
    int n= 0;
    if( argc > 1) n = atoi(argv[1]);
    float *y = malloc(sizeof(float) * n);
    rand_init(n,y);
    print(1,n,y);
    sort(n,y);
    print(1,n,y);

    return 0;
}
