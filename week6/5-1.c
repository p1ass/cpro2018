/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/
#include <stdio.h>
#include <stdlib.h>

//プロトタイプ宣言
int input(char str);
int cnr(int n,int r);

int main(void){
    int n = input('n');
    int r = input('r');
    int ans = cnr(n,r);
    printf("%d",ans);
    return 0;
}

int input(char str){
    int count = 0;
    int n = 0;
    int error = 0;

    do{
        printf("%c = ",str);
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

int cnr(int n,int r) {
    if(r == 0 || (n-r == 0)){
        return 1;
    }else{
        return cnr(n-1,r-1) + cnr(n-1,r);
    }
}