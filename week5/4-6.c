/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int hist(int n);

int main() {
    srand(time(NULL));
    hist(10);
    hist(100);
    hist(1000);
    hist(10000);
    hist(100000);
    hist(1000000);
    return 0;
}

int hist(int n){
    float counter[10] = {0};
    int i = 0;
    int num = 0;

    //例外処理
    if(n<=0){
        printf("Invalid n = %d \n",n);
        return -1;
    }

    //表示回数を配列に保存
    while(i<n){
        num = rand() % 10;
        counter[num] += 1;
        i++;
    }

    //表示回数を%表記に直す
    for(i=0;i<10;i++){
        counter[i] /= n;
        counter[i] *= 100;
    }

    printf("n = %7d  ",n);

    for(i=0;i<10;i++){
        printf("%4.1f ",counter[i]);
    }
    printf("\n");
    return 0;
}
