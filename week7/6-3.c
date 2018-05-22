/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>

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


void add(int n, const float *x,float *o){
    int i;

    for(i=0;i<n;i++){
        o[i] += x[i];
    }
}
int main()
{
    float x[6] = {0, 1, 2, 3, 4, 5};
    float y[6] = {1, 1, 2, 3, 5, 8};
    print_oct(2, 3, x, "x");
    print_oct(2, 3, y, "y");
    add(6, x, y);
    print_oct(2, 3, y, "y");
    return 0;
}