/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>

void fc(int m,int n,const float *x, const float *A, const float *b, float *o){
    int i,j;

    for(i=0;i<m;i++){
        o[i] = 0.0;
        for(j=0;j<n;j++){
            o[i] += A[i*n+j] * x[j];
        }
        o[i] += b[i];
    }
}

void print_oct(int m, int n, const float *x, const char *name)
{
    int i, j;

    printf("%s = [", name);
    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
        {
            printf("%.4f ", x[i * n + j]);
        }
        printf(";\n");
    }
    printf("];\n");
}

int main(){
    float A[6] = {1, 2, 3, 4, 5, 6};
    float b[2] = {0.5, 0.25};
    float x[3] = {2, 3, 5};
    float o[2];
    fc(2, 3, x, A, b, o);
    print_oct(2, 3, A, "A");
    print_oct(2, 1, b, "b");
    print_oct(3, 1, x, "x");
    print_oct(2, 1, o, "o");
    return 0;
}