#include "nn.h"
#include <math.h>

//プロトタイプ宣言
void print(int m,int n,const float *x);
void fc(int m,int n,const float *A,const float *b,const float *x, float *y);
void ReLU(int n,const float *x, float *y);
void softmax(int n,const float *x, float *y);
int inference3(const float *A, const float *b, const float *x);


int main() {
    float * train_x = NULL;
    unsigned char * train_y = NULL;
    int train_count = -1;

    float * test_x = NULL;
    unsigned char * test_y = NULL;
    int test_count = -1;

    int width = -1;
    int height = -1;
    printf("load_mnist\n");
    load_mnist(&train_x, &train_y, &train_count,
             &test_x, &test_y, &test_count,
             &width, &height);

  // これ以降，３層NNの係数 A_784x10 および b_784x10 と，
  // 訓練データ train_x[0]～train_x[train_count-1], train_y[0]～train_x[train_count-1],
  // テストデータ test_x[0]～test_x[test_count-1], test_y[0]～test_y[test_count-1],
  // を使用することができる．

  //float *y = malloc(sizeof(float)*10);

  int ans;
  ans = inference3(A_784x10,b_784x10,train_x);
  printf("%d %d\n",ans,train_y[0]);
  return 0;
}


//行列を表示
void print(int m,int n,const float *x){
    int i,j;

    for(i=0;i<m;i++){
        for(j=0;j<n;j++){
            printf("%.4f ",x[i*n+j]);
        }
        printf("\n");
    }
}

//y = A*x +b を計算する
void fc(int m,int n,const float *A, const float *b, const float *x, float *y){
    int i,j;

    for(i=0;i<m;i++){
        y[i] = 0;
        for(j=0;j<n;j++){
            y[i] += A[i*n+j] * x[j];
        }
        y[i] += b[i];
    }
}

//ReLU演算を行う y = ReLU(x)
void ReLU(int n,const float *x, float *y){
    int i;

    for(i=0;i<n;i++){
        if(x[i] > 0){
            y[i] = x[i];
        }else{
            y[i] = 0;
        }
    }
}

void softmax(int n,const float *x, float *y){
    int i;
    float max = x[0]; //xの最大値
    float sum = 0; //sigma(exp(x-max))

    //xの最大値を求める
    for(i=0;i<n;i++){
        if(x[i] > max){
            max = x[i];
        }
    }

    //sigma(exp(x-max))を計算
    for(i=0;i<n;i++){
        sum += exp(x[i] - max);
    }

    //softmax演算を行う
    for(i=0;i<n;i++){
        y[i] = exp(x[i] -max) / sum;
    }
}

//3層による推論
int inference3(const float *A, const float *b, const float *x){
    int m = 784;
    int n = 10;
    float *y = malloc(sizeof(float)*10);
    // float y[10];

    fc(m,n,A,b,x,y);
    ReLU(n,y,y);
    softmax(n,y,y);

    float max = y[0]; //yの要素の最大のもの
    int index = 0; //yの要素が最大の時の添字
    int i;

    for(i=0;i<m;i++){
        if(y[i] > max){
            max = y[i];
            index = i;
        }
    }
    return index;
}
