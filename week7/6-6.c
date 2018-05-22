/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void rand_init(int n, float *o){
    int i;

    for(i=0;i<n;i++){
        o[i] = (rand() * 2 / (1.0 + RAND_MAX)) - 1.0;
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
    srand(time(NULL));
    float y[6];
    // 初期化前の値
    print_oct(2, 3, y, "y");
    rand_init(6, y);
    // 初期化後の値
    print_oct(2, 3, y, "y");
    return 0;
}