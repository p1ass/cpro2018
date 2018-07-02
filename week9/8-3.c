/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/
#include <stdio.h>
#include <math.h>

typedef struct{
    double x;
    double y;

}Vector2d;

double GetLength(Vector2d *vector ){
    return sqrt(pow(vector->x,2) + pow(vector->y,2));
}

void ScaleVector(Vector2d *v,double s){
    v->x *= s;
    v->y *= s;
}

int main(void){
    Vector2d vec;
    double scale;

    printf("Input 2D Vector : ");
    scanf("%lf %lf",&vec.x,&vec.y);

    printf("Input scale value : ");
    scanf("%lf",&scale);

    ScaleVector(&vec,scale);
    double len = GetLength(&vec) ;

    printf("Result : %f %f\n",vec.x,vec.y);
    printf("Length : %f\n",len);

    return 0;
}
