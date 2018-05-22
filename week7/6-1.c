/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>

void print(int m, int n, const float *x){
    int i, j;

    for (i = 0; i < m; i++){
        for (j = 0; j < n; j++){
            printf("%.4f ", x[i * n + j]);
        }
        printf("\n");
    }
}
int main()
{
    float m[12] = {0, 1, 2, 3, 4, 5,
                   6, 7, 8, 9, 10, 11};
    print(3, 4, m);
    return 0;
}
