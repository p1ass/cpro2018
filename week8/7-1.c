/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>

int main(void){
    char str[128];
    int len = 0;
    printf("Input a word :");
    scanf("%s",str);
    while(str[len]){
        len++;
    }

    int i;
    for(i=0;i<len;i++){
        if(str[i] == 'Z'){
            printf("A");
        }else if(str[i] == 'z'){
            printf("a");
        }else{
            printf("%c",str[i]+1);
        }
    }
    printf("\n");

    return 0;
}
