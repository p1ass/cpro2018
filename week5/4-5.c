/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/
#include <stdio.h>
#include <stdlib.h>

//プロトタイプ宣言
int input(void);
int fact(int n);
int perm(int n,int r);

int main(void){
    int n = input();

    int i;
    int p;
    for(i=0;i<=n;i++){
        p = perm(n,i);
        printf("perm(%d,%d) = %d\n",n,i,p);
    }
    return 0;
}

int input(void){
    int count = 0;
    int n = 0;
    int error = 0;

    do{
        printf("n = ");
        count = scanf("%d",&n);

        //エラー処理
        if(count != 1){
            scanf("%*s");
            error = 1;
        }else if(n < 0){
            error = 1;
        }else{
            error = 0;
        }

        if(error){
            printf("Invalid input\n");
        }
    }while(error);

    return n;
}

int fact(int n){
    int i;
    int factorial = 1;

    if(n<0){
        printf("Invalid n\n");
        return -1;
    }

    for(i=1;i<=n;i++){
        factorial *=i;
    }

    return factorial;
}

int perm(int n,int r){
    return fact(n) / fact(n-r);
}
