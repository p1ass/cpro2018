/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

/*
float型は4バイトであり、精度は10進数で約7桁ほどである。
対して、double型は8バイトであり、精度は10進数で約15桁ほどである。
したがって、1e-8を足す操作では、誤差が積み重なり、float型の方がdouble型に比べて低い精度の値が出てしまう。
また、掛け算は指数部と仮数部分けて行われるため、1e-8*1e+8という操作ではその誤差が生じないため、float型、double型どちらにおいても正しい値が算出される。

参考 : http://www.cc.kyoto-su.ac.jp/~yamada/programming/float.html
*/
#include <stdio.h>
int main(void){
    float a,b = 0;
    double c,d = 0;

    int i;
    for(i=0;i<1e+8;i++){
        a += 1e-8;
        c += 1e-8;
    }

    b = 1e-8*1e+8;
    d = 1e-8*1e+8;

    printf("a = %.20f\n",a);
    printf("b = %.20f\n",b);
    printf("c = %.20lf\n",c);
    printf("d = %.20lf\n",d);

    return 0;
}