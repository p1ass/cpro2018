/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include "nn.h"
#include "nn_module.h"
#include <time.h>

/*
第1~3引数 : FC1~FC3のパラメータの読み込み先
第4引数 : 推論する画像
*/
int main(int argc, char *argv[]){

    //パラメータの保存先が指定されていない場合エラーを返す
    if (argc <5){
        printf("データの保存先を引数で設定してください。\n");
        return -1;
    }
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
    float *A1 = malloc(sizeof(float)* 784*50);
    float *A2 = malloc(sizeof(float)* 50*100);
    float *A3 = malloc(sizeof(float)* 100*10);
    float *b1 = malloc(sizeof(float)*50);
    float *b2 = malloc(sizeof(float)*100);
    float *b3 = malloc(sizeof(float)*10);
    float *y = malloc(sizeof(float) * 10);

    //パラメータを保存
    load(argv[1], 50, 784, A1, b1);
    load(argv[2], 100, 50, A2, b2);
    load(argv[3], 10, 100, A3, b3);

    //画像を読み込む
    float *x = load_mnist_bmp(argv[4]);

    //読み込んだ画像で推論
    int  predict = 0;
    predict = inference6(A1, A2, A3, b1, b2, b3, x,y);

    printf("読み込んだ画像は%dです。\n",predict);
    return 0;
}
