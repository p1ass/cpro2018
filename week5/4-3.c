/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/
#include <stdio.h>

/* 関数absを自分で作成 */
float abs(float num){
    if(num>=0){
        return num;
    }else{
        return -1*num;
    }

}

int main() {
    float x = -1.3;
    float y = 3.7;
    float abs_x = abs(x);
    float abs_y = abs(y);
    printf("|x|=%f\n", abs_x);
    printf("|y|=%f\n", abs_y);
    return 0;
}