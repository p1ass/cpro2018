/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/
#include <stdio.h>

void swap(int *pa, int *pb){
    int temp = *pa;
    *pa = *pb;
    *pb = temp;
}

int main(void){
    int data[] = {64,30,8,87,45,13};
    int n = 6;
    int i,j,k;

    for(i=0;i<n;i++){
        for(j=0;j<n-i-1;j++){
            if(data[j]>data[j+1]){
                swap(&data[j],&data[j+1]);
            }
        }
        printf("loop%d : ",i+1);
        for(k=0;k<n;k++){
            printf("%d ",data[k]);
        }
        printf("\n");
    }

    return 0;
}
