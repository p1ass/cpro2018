/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>
#include <math.h>

int main(void){
    double a,b,c,d = 0;
    printf("a = ");
    scanf("%lf",&a);
    printf("b = ");
    scanf("%lf",&b);
    printf("c = ");
    scanf("%lf",&c);

    d = pow(b,2)-4*a*c;

    if(d >= 0){
        double x1,x2;
        x1 = (-b + sqrt(d)) / 2*a;
        x2 = (-b - sqrt(d)) / 2*a;

        printf("%f \n%f\n",x1,x2);
    }else{
        double re,im;
        re = -b / 2*a;
        im = sqrt(-d) / 2*a;

        printf("%f+i%f\n%f-i%f\n",re,im,re,im);
    }

    return 0;
}
