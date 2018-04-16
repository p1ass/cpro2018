/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>
#include <math.h>

int main(void){
    double a,b,c;

    double x1,x2;

    printf("solve ax^2+bx+c=0\n\n");
    printf("please input \"a\"\na = ");
    scanf("%lf",&a);
    printf("please input \"b\"\nb = ");
    scanf("%lf",&b);
    printf("please input \"c\"\nc = ");
    scanf("%lf",&c);

    x1 = (-b + sqrt(pow(b,2)-4*a*c)) / 2*a; 
    x2 = (-b - sqrt(pow(b,2)-4*a*c)) / 2*a; 

    printf("Answers are\n %.3f and %.3f.",x1,x2);

    return 0;
}