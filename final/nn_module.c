/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//m*n行列xを表示
void print(int m, int n, const float *x){
    int i, j;

    for (i = 0; i < m; i++){
        for (j = 0; j < n; j++){
            printf("%.4f ", x[i * n + j]);
        }
        printf("\n");
    }
}

//m*n行列xをyにコピーする
void copy(int m, int n, const float *x, float *y){
    int i, j;
    for (i = 0; i < m; i++){
        for (j = 0; j < n; j++){
            y[i * n + j] = x[i * n + j];
        }
    }
}

//m*n行列Aを用いてy = A*x +b を計算する
void fc(int m, int n, const float *x, const float *A, const float *b, float *y){
    int i, j;

    for (i = 0; i < m; i++){
        y[i] = b[i];
        for (j = 0; j < n; j++){
            y[i] += A[i * n + j] * x[j];
        }
    }
}

//m列ベクトルのxに対して、relu演算を行う y = relu(x)
void relu(int m, const float *x, float *y){
    int i;

    for (i = 0; i < m; i++){
        if (x[i] > 0){
            y[i] = x[i];
        }
        else{
            y[i] = 0;
        }
    }
}

//m列ベクトルのxに対して、prelu演算を行う y = prelu(x)
void prelu(int m, const float *x, float *y,float a){
    int i;

    for (i = 0; i < m; i++){
        if (x[i] > 0){
            y[i] = x[i];
        }
        else{
            y[i] = a * x[i];
        }
    }
}

//m行のベクトルxに対して、y = softmax(x)
void softmax(int m, const float *x, float *y){
    int i;
    float max = x[0]; //xの最大値
    float sum = 0;    //sigma(exp(x-max))

    //xの最大値を求める
    for (i = 0; i < m; i++){
        if (x[i] > max){
            max = x[i];
        }
    }

    //sigma(exp(x-max))を計算
    for (i = 0; i < m; i++)
    {
        sum += exp(x[i] - max);
    }

    //softmax演算を行う
    for (i = 0; i < m; i++)
    {
        y[i] = exp(x[i] - max) / sum;
    }
}

//3層による推論を行い、得られた結果[0:9]を返す
int inference3(const float *A, const float *b, const float *x,float * y){
    int m = 10;
    int n = 784;

    fc(m, n, x, A, b, y);
    relu(m, y, y);
    softmax(m, y, y);

    float max = y[0]; //yの要素の最大のもの
    int index = 0;    //yの要素が最大の時の添字
    int i;

    for (i = 0; i < m; i++){
        if (y[i] > max){
            max = y[i];
            index = i;
        }
    }
    return index;
}

//6層による推論を行い、得られた結果[0:9]を返す
int inference6(const float *A1,const float *A2,const float *A3, const float *b1,  const float *b2, const float *b3, const float *x,float * y){

    float * y1 = malloc(sizeof(float) * 50);
    float * y2 = malloc(sizeof(float) * 100);
    fc(50, 784, x, A1, b1, y1);
    relu(50, y1, y1);
    fc(100, 50, y1, A2, b2, y2);
    relu(100, y2, y2);
    fc(10, 100, y2, A3, b3, y);
    softmax(10, y, y);

    float max = y[0]; //yの要素の最大のもの
    int index = 0;    //yの要素が最大の時の添字
    int i;

    for (i = 0; i < 10; i++){
        if (y[i] > max){
            max = y[i];
            index = i;
        }
    }
    free(y1);
    free(y2);
    return index;
}

//softmaxの誤差逆伝播を行う
void softmaxwithloss_bwd(int m, const float *y, unsigned char t, float *dEdx){
    float answers[10] = {0};
    int i = 0;

    for (i = 0; i < 10; i++){
        if (i == t){
            answers[i] = 1;
        }
    }
    for (i = 0; i < m; i++){
        dEdx[i] = y[i] - answers[i];
    }
}

//reluの誤差逆伝播を行う
void relu_bwd(int m, const float *x, const float *dEdy, float *dEdx){
    int i = 0;
    for (i = 0; i < m; i++){
        if (x[i] > 0){
            dEdx[i] = dEdy[i];
        }
        else{
            dEdx[i] = 0;
        }
    }
}

//reluの誤差逆伝播を行う
void prelu_bwd(int m, const float *x, const float *dEdy, float *dEdx,const float a, float  dEda){
    int i = 0;
    dEda = 0.0;
    for (i = 0; i < m; i++){
        if (x[i] > 0){
            dEdx[i] = dEdy[i];
            dEda += 0.0;
        }
        else{
            dEdx[i] = a;
            dEda += dEdy[i] * x[i];
        }
    }
}

//FC層の誤差逆伝播を行う
void fc_bwd(int m, int n, const float *x, const float *dEdy, const float *A, float *dEdA, float *dEdb, float *dEdx){
   int i, j = 0;

   for (i = 0; i < m; i++){
       for (j = 0; j < n; j++){
           dEdA[i * n + j] = dEdy[i] * x[j];
       }
   }

   for (i = 0; i < m; i++){
       dEdb[i] = dEdy[i];
   }

   for (i = 0; i < n; i++){
       dEdx[i] = 0;
       for (j = 0; j < m; j++){
           dEdx[i] += A[j * m + i] * dEdy[j];
       }
   }
}

//3層の誤差逆伝播を行う
void backward3(const float *A, const float *b, const float *x, unsigned char t, float *y, float *dEdA, float *dEdb){
    int m = 10;
    int n = 784;

    float before_fc[784];
    float before_relu[10];
    float y1[10];
    float y0[784];
    //順伝播
    copy(1, n, x, before_fc);
    fc(m, n, x, A, b, y);
    copy(1, m, y, before_relu);
    relu(m, y, y);
    softmax(m, y, y);

    //逆伝播
    softmaxwithloss_bwd(m, y, t, y1);
    relu_bwd(m, before_relu, y1, y1);
    fc_bwd(m, n, before_fc, y1, A, dEdA, dEdb, y0);
}

//6層の誤差逆伝播を行う
void backward6(const float *A1,const float *A2,const float *A3, const float *b1,  const float *b2, const float *b3, const float *x, unsigned char t, float *y,
    float *dEdA1,float *dEdA2,float *dEdA3, float *dEdb1,float *dEdb2,float *dEdb3){

    float  * before_fc1 = malloc(sizeof(float) * 784);
    float  * before_fc2 = malloc(sizeof(float) * 50);
    float  * before_fc3 = malloc(sizeof(float) * 100);
    float  * before_relu1 = malloc(sizeof(float) * 50);
    float  * before_relu2 = malloc(sizeof(float) * 100);

    float * y0 = malloc(sizeof(float) * 784);
    float * y1 = malloc(sizeof(float) * 50);
    float * y2 = malloc(sizeof(float) * 100);
    float *y3 = malloc(sizeof(float) * 10);

    //順伝播
    copy(1,784,x,before_fc1);
    fc(50, 784, x, A1, b1, before_relu1);
    relu(50, before_relu1, before_fc2);
    fc(100, 50, before_fc2, A2, b2, before_relu2);
    relu(100, before_relu2, before_fc3);
    fc(10, 100, before_fc3, A3, b3, y);
    softmax(10, y, y);

    //逆伝播
    softmaxwithloss_bwd(10, y, t, y3);
    fc_bwd(10, 100, before_fc3, y3, A3, dEdA3, dEdb3, y2);
    relu_bwd(100, before_relu2, y2, y2);
    fc_bwd(100, 50, before_fc2, y2, A2, dEdA2, dEdb2, y1);
    relu_bwd(50, before_relu1, y1, y1);
    fc_bwd(50, 784, before_fc1, y1, A1, dEdA1, dEdb1, y0);

    //メモリ解放
    free(y0);free(y1);free(y2);free(y3);
    free(before_fc1);free(before_fc2);free(before_fc3);
    free(before_relu1);free(before_relu2);
}

//n行ベクトルxの配列をシャッフルする
void shuffle(int n, int *x){
    int i, j;
    for (i = 0; i < n; i++){
        j = (int)(rand()*(  n+1.0)/(1.0+RAND_MAX));
        int tmp_xi = x[i];
        x[i] = x[j];
        x[j] = tmp_xi;
    }
}

//損失関数を求める
float cross_entropy_error(const float * y, int t){
    return -1 *log(y[t] + 1e-7);
}

//n行ベクトルxをoに足す
void add(int n, const float *x, float *o){
    int i;
    for (i = 0; i < n; i++){
        o[i] += x[i];
    }
}

//n行ベクトルoにxをかける
void scale(int n, float x, float *o){
    int i;
    for (i = 0; i < n; i++){
        o[i] *= x;
    }
}

//n行ベクトルoをxで全要素を初期化
void init(int n, float x, float *o){
    int i;
    for (i = 0; i < n; i++){
        o[i] = x;
    }
}

//[-1:1]の範囲でn行ベクトルoを初期化
void rand_init(int n, float *o){
    int i;
    for (i = 0; i < n; i++){
        o[i] = ((float)rand() / ((float)RAND_MAX + 1)) * 2 - 1 ;
    }
}

//[0:1]の一様乱数を生成
//http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html#Gauss
float uniform( void ){
    return rand()/(RAND_MAX+1.0);
}

//n行ベクトルを正規分布で初期化
void rand_init_by_normal_dist(int n, float *o,float mu,float sigma){
    int i;
    for(i=0;i<n;i++){
        float z =sqrt( -2.0*log(uniform()) ) * cos( 2.0*M_PI*uniform() );
        o[i] =  sigma * z + mu;
        // printf("%f",o[i]);
    }
}


//一層分のパラメータを保存
void save(const char * filename, int m, int n, const float * A, const float * b){
    FILE *fp;
    fp = fopen(filename,"w");

    fwrite(A, sizeof(float), m*n, fp);
    fwrite(b, sizeof(float), m, fp);

    fclose(fp);
}

//一層分のパラメータを読み込む
void load(const char * filename, int m, int n, float * A, float * b){
    FILE *fp;
    fp = fopen(filename,"r");

    fread(A, sizeof(float), m*n, fp);
    fread(b, sizeof(float), m, fp);

    fclose(fp);
}
