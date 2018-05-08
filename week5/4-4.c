/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/
#include <stdio.h>
#include <stdlib.h>

#define M 3
#define N 4

int a[M][N];

int initialize_array(void){
    int i,j;

    for(i=0; i<M; i++) {
      for(j=0; j<N; j++) {
          a[i][j] = 10*i+j;
      }
    }
    return 0;
}

int show_array(void){
    int i, j;

    for(i=0; i<M; i++) {
      for(j=0; j<N; j++) {
          printf("%2d ",a[i][j]);
      }
      printf("\n");
  }
  return 0;
}

int main(void) {
    initialize_array();
    show_array();
    return 0;
}
