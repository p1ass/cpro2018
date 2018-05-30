/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>

int main(void){

    FILE *r_fp, *w_fp;
    char str[128];

    r_fp = fopen("test.txt","r");
    if(!r_fp){
        printf("File cannot open\n");
        return -1;
    }

    w_fp = fopen("test.bak","w");

    while(fgets(str,128,r_fp)){
        fprintf(w_fp,"%s\n",str);
    }

    fclose(r_fp);
    fclose(w_fp);
    return 0;
}
