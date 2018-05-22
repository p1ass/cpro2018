#include "nn.h"
#include <time.h>

//プロトタイプ宣言
void print(int m,int n,const float *x);
void copy(int m, int n, const float *x, float *y);
void fc(int m,int n,const float *A,const float *b,const float *x, float *y);
void ReLU(int n,const float *x, float *y);
void softmax(int n,const float *x, float *y);
int inference3(const float *A, const float *b, const float *x,float * y);
void softmaxwithloss_bwd(int n, const float *y, unsigned char t, float *dEdx);
void relu_bwd(int n, const float * x, const float * dEdy, float * dEdx);
void fc_bwd(int m, int n, const float * x, const float * dEdy, const float * A, float * dEdA, float * dEdb, float * dEdx);
void backward3(const float *A, const float *b, const float *x, unsigned char t, float *y, float *dEdA, float *dEdb);
void shuffle(int n,int * x);
float cross_entropy_error(const float * y, int t);
void add(int n, const float *x, float *o);
void scale(int n, float x, float *o);
void init(int n, float x, float *o);
void rand_init(int n, float *o);

int main()
{
    float *train_x = NULL;
    unsigned char *train_y = NULL;
    int train_count = -1;

    float *test_x = NULL;
    unsigned char *test_y = NULL;
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

    //初期化
    srand(time(NULL));
    float *y = malloc(sizeof(float) * 10);
    float *dEdA = malloc(sizeof(float) * 784 * 10);
    float *dEdb = malloc(sizeof(float) * 10);
    float *dEdA_av = malloc(sizeof(float) * 784 * 10);//平均
    float *dEdb_av = malloc(sizeof(float) * 10);//平均
    float *A = malloc(sizeof(float)* 784*10);
    float *b = malloc(sizeof(float)*10);
    int * index = malloc(sizeof(int)*train_count);
    int epoch = 20;
    int batch = 100;
    float h = 0.01;
    int i,j,k,l,m;
    float acc = 0.0;

    rand_init(784*10,A);
    rand_init(10,b);
    //エポックを回す
    for(i=0;i<epoch;i++){
        printf("Epoch %d/%d\n", i+1,epoch);
        //インデックスを作成し、並び替え

        for(l=0 ; l<train_count ; l+=1) {
            index[l] = l;
            // printf("%d\n",index[i]);
        }

        shuffle(train_count, index);

        //ミニバッチ
        for(j=0;j<train_count /batch;j++){
            // printf("Mini batch %d\n",j+1);
            //平均勾配を初期化
            init(784*10,0,dEdA_av);
            init(10,0,dEdb_av);

            //一つ一つの勾配を計算する
            for(k=0;k<batch;k++){
                backward3(A,b,train_x+ 784*index[j*batch+k] ,train_y[index[j*batch+k]],y,dEdA,dEdb);
                add(784*10,dEdA,dEdA_av);
                add(10,dEdb,dEdb_av);
            }
            // print(1,10,dEdb_av);

            // print(1,10,dEdb_av);
            //ミニバッチで割って平均を求める
            scale(784*10,-h/batch,dEdA_av);
            scale(10,-h/batch,dEdb_av);
            // print(1,10,dEdb_av);

            //係数A,bを更新
            add(784*10,dEdA_av,A);
            add(10,dEdb_av,b);


        }

        //テストデータでの推論
        int sum = 0;
        float loss_sum = 0;
        // printf("test_count : %d\n",test_count);
        for(m=0 ; m<test_count ; m++) {
            if(inference3(A, b, test_x + m*width*height,y) == test_y[m]) {
                sum++;
            }
            loss_sum += cross_entropy_error(y,test_y[m]);

        }
        acc = sum * 100.0 / test_count;

        printf("Accuracy : %f ％ \n",acc);
        printf("Loss Average : %f\n",loss_sum/test_count);
    }

    return 0;
}

    //行列を表示
    void print(int m, int n, const float *x){
        int i, j;

        for (i = 0; i < m; i++){
            for (j = 0; j < n; j++){
                printf("%.4f ", x[i * n + j]);
            }
            printf("\n");
        }
    }

    //xをyにコピーする
    void copy(int m, int n, const float *x, float *y){
        int i, j;
        for (i = 0; i < m; i++){
            for (j = 0; j < n; j++){
                y[i * n + j] = x[i * n + j];
            }
        }
    }

    //y = A*x +b を計算する
    void fc(int m, int n, const float *A, const float *b, const float *x, float *y){
        int i, j;

        for (i = 0; i < n; i++){
            y[i] = 0.0;
            for (j = 0; j < m; j++){
                y[i] += A[i * m + j] * x[j];
            }
            y[i] += b[i];
        }
    }

    //ReLU演算を行う y = ReLU(x)
    void ReLU(int n, const float *x, float *y){
        int i;

        for (i = 0; i < n; i++){
            if (x[i] > 0){
                y[i] = x[i];
            }
            else{
                y[i] = 0;
            }
        }
    }

    //softmax
    void softmax(int n, const float *x, float *y){
        int i;
        float max = x[0]; //xの最大値
        float sum = 0;    //sigma(exp(x-max))

        //xの最大値を求める
        for (i = 0; i < n; i++){
            if (x[i] > max){
                max = x[i];
            }
        }

        //sigma(exp(x-max))を計算
        for (i = 0; i < n; i++)
        {
            sum += exp(x[i] - max);
        }

        //softmax演算を行う
        for (i = 0; i < n; i++)
        {
            y[i] = exp(x[i] - max) / sum;
        }
    }

    //3層による推論
    int inference3(const float *A, const float *b, const float *x,float * y){
        int m = 784;
        int n = 10;
        // float *y = malloc(sizeof(float) * 10);
        // float y[10];

        fc(m, n, A, b, x, y);
        ReLU(n, y, y);
        softmax(n, y, y);

        float max = y[0]; //yの要素の最大のもの
        int index = 0;    //yの要素が最大の時の添字
        int i;

        for (i = 0; i < n; i++){
            if (y[i] > max){
                max = y[i];
                index = i;
            }
        }
        // free(y);
        return index;
    }

    //softmaxの逆誤差伝播
    void softmaxwithloss_bwd(int n, const float *y, unsigned char t, float *dEdx){
        float answers[10] = {0};
        int i = 0;

        for (i = 0; i < 10; i++){
            if (i == t){
                answers[i] = 1;
            }
        }
        for (i = 0; i < n; i++){
            dEdx[i] = y[i] - answers[i];
        }
    }

    //reluの逆誤差伝播
    void relu_bwd(int n, const float *x, const float *dEdy, float *dEdx){
        int i = 0;
        for (i = 0; i < n; i++){
            if (x[i] > 0){
                dEdx[i] = dEdy[i];
            }
            else{
                dEdx[i] = 0;
            }
        }
    }

    //FC層の逆誤差伝播
    void fc_bwd(int m, int n, const float *x, const float *dEdy, const float *A, float *dEdA, float *dEdb, float *dEdx){
        int i, j = 0;

        for (i = 0; i < n; i++){
            for (j = 0; j < m; j++){
                // dEdA[j*n+i] = 0;
                dEdA[j * n + i] = dEdy[i] * x[i];
            }
        }

        for (i = 0; i < m; i++){
            dEdb[i] = dEdy[i];
        }

        for (i = 0; i < n; i++){
            dEdx[i] = 0;
            for (j = 0; j < m; j++){
                dEdx[i] += A[j * n + i] + dEdy[j];
            }
        }
    }

    //3層の逆誤差伝播
    void backward3(const float *A, const float *b, const float *x, unsigned char t, float *y, float *dEdA, float *dEdb){
        int m = 784;
        int n = 10;

        float before_fc[784];
        float before_relu[10];
        //順伝播
        copy(1, m, x, before_fc);
        fc(m, n, A, b, x, y);
        copy(1, n, y, before_relu);
        ReLU(n, y, y);
        softmax(n, y, y);

        //逆伝播
        softmaxwithloss_bwd(n, y, t, y);
        relu_bwd(n, before_relu, y, y);
        fc_bwd(m, n, before_fc, y, A, dEdA, dEdb, y);
    }

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

    void add(int n, const float *x, float *o){
        int i;

        for (i = 0; i < n; i++){
            o[i] += x[i];
        }
    }

    void scale(int n, float x, float *o){
        int i;

        for (i = 0; i < n; i++){
            o[i] *= x;
        }
    }

    void init(int n, float x, float *o){
        int i;

        for (i = 0; i < n; i++){
            o[i] = x;
        }
    }

    void rand_init(int n, float *o){
        int i;

        for (i = 0; i < n; i++){
            o[i] = ((float)rand() / ((float)RAND_MAX + 1)) * 2 - 1 ;
        }
    }
