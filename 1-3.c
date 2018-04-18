/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>
#include <math.h>

int main(void){
    double a,b,c;

    double x1,x2;

    printf("a = ");
    scanf("%lf",&a);
    printf("b = ");
    scanf("%lf",&b);
    printf("c = ");
    scanf("%lf",&c);

    x1 = (-b + sqrt(pow(b,2)-4*a*c)) / 2*a;
    x2 = (-b - sqrt(pow(b,2)-4*a*c)) / 2*a;

    printf("%f \n%f\n",x1,x2);

    return 0;
}
