/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/
#include <stdio.h>

int input(char str);
int print_bit(int n);

int main(void){
    int n = input('n');
    print_bit(n);
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
        }else{
            error = 0;
        }

        if(error){
            printf("Invalid input\n");
        }
    }while(error);

    return n;
}

int print_bit(int n){
    int i;
    printf("%d(10) = ",n);

    for(i=31;i>=0;i--){
       printf("%d",(n>>i) & 1);
    }
    printf(" (2)\n");
    return 0;
}