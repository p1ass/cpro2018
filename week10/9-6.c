/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


void print(int m, int n, const float *x){
    int i, j;

    for (i = 0; i < m; i++){
        for (j = 0; j < n; j++){
            printf("%.4f ", x[i * n + j]);
        }
        printf("\n");
    }
}

void rand_init(int n, float *o){
    int i;

    for (i = 0; i < n; i++){
        o[i] = ((float)rand() / ((float)RAND_MAX + 1)) * 2 - 1 ;
    }
}

void init(int n, float x, float *o){
    int i;

    for (i = 0; i < n; i++){
        o[i] = x;
    }
}

void save(const char *filename, int m,int n,const float *A, const float *b){
    FILE *fp;
    fp = fopen(filename,"w");

    fwrite(A, sizeof(float), m*n, fp);
    fwrite(b, sizeof(float), m, fp);

    fclose(fp);
}

void load(const char *filename,int m,int n,float *A,float *b){
    FILE *fp;
    fp = fopen(filename,"r");

    fread(A, sizeof(float), m*n, fp);
    fread(b, sizeof(float), m, fp);

    fclose(fp);
}

int main(int argc, char *argv[]) {
    int m,n= 0;
    if( argc > 1) m = atoi(argv[1]);
    if( argc > 2) n = atoi(argv[2]);

    float *A = malloc(sizeof(float) * m*n);
    float *b = malloc(sizeof(float) *m);
    
    init(m*n,1,A);
    init(m,2,b);


    save("test.dat",m,n,A,b);
    load("test.dat",m,n,A,b);

    print(1,m*n,A);
    print(1,m,b);
    return 0;
}
