/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>
int main(void){
    char str = 'a';
    printf("%c,%d,%x\n",str,str,str);
    str+= 1;
    printf("%c,%d,%x\n",str,str,str);
    return 0;
}