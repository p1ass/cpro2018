/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include "nn.h"
#include "nn_module.h"
#include <time.h>

//プロトタイプ宣言
void learn_3layers(int train_count,int test_count,float * train_x,unsigned char * train_y,float * test_x, unsigned char * test_y );

int main(){

    float *train_x = NULL;
    unsigned char *train_y = NULL;
    int train_count = -1;

    float *test_x = NULL;
    unsigned char *test_y = NULL;
    int test_count = -1;

    int width = -1;
    int height = -1;

    load_mnist(&train_x, &train_y, &train_count,
               &test_x, &test_y, &test_count,
               &width, &height);

    //初期化
    srand(time(NULL));

    learn_3layers(train_count,test_count,train_x,train_y,test_x,test_y);
    return 0;
}
// 3層での学習
void learn_3layers(int train_count,int test_count,float * train_x,unsigned char * train_y,float * test_x, unsigned char * test_y ){
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
    float h = 0.01 ;
    int i,j,k,l,m;

    rand_init(784*10,A);
    rand_init(10,b);
    //エポックを回す
    for(i=0;i<epoch;i++){
        printf("-----------------------------------------------------\n");
        printf("Epoch %d/%d\n", i+1,epoch);
        //インデックスを作成し、並び替え

        for(l=0 ; l<train_count ; l+=1) {
            index[l] = l;
            // printf("%d\n",index[i]);
        }

        shuffle(train_count, index);

        //ミニバッチ
        for(j=0;j<train_count /batch;j++){
            //平均勾配を初期化
            init(784*10,0,dEdA_av);
            init(10,0,dEdb_av);

            //一つ一つの勾配を計算する
            for(k=0;k<batch;k++){
                backward3(A,b,train_x+ 784*index[j*batch+k] ,train_y[index[j*batch+k]],y,dEdA,dEdb);
                add(784*10,dEdA,dEdA_av);
                add(10,dEdb,dEdb_av);
            }
            //ミニバッチで割って平均を求める
            scale(784*10,-h/batch,dEdA_av);
            scale(10,-h/batch,dEdb_av);
            // print(1,10,dEdb_av);

            //係数A,bを更新
            add(784*10,dEdA_av,A);
            add(10,dEdb_av,b);
            // print(784,10,dEdA_av);

        }

        //訓練データでの推論
        int sum = 0;
        float loss_sum = 0;
        float acc = 0.0;
        for(m=0 ; m<train_count ; m++) {
            if(inference3(A,b, train_x + m*784,y) == train_y[m]) {
                sum++;
            }
            loss_sum += cross_entropy_error(y,train_y[m]);
        }
        acc = sum * 100.0 / train_count;

        printf("Accuracy : %f ％ \n",acc);
        printf("Loss  Average: %f\n",loss_sum/train_count);

        //テストデータでの推論
        int sum_test = 0;
        float loss_sum_test = 0;
        float acc_test = 0.0;
        for(m=0 ; m<test_count ; m++) {
            if(inference3(A, b, test_x + m*784,y) == test_y[m]) {
                sum_test++;
            }
            loss_sum_test += cross_entropy_error(y,test_y[m]);
        }
        acc_test = sum_test * 100.0 / test_count;

        printf("Accuracy Val : %f ％ \n",acc_test);
        printf("Loss  Average Val : %f\n",loss_sum_test/test_count);
    }
}
