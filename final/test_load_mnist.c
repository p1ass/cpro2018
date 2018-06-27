#include "nn.h"
#include "util.h"
#include <time.h>

int main(int argc, char *argv[]){

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
    load("fc1.dat", 50, 784, A1, b1);
    load("fc2.dat", 100, 50, A2, b2);
    load("fc3.dat", 10, 100, A3, b3);

    // 画像を保存
    int i = 4572;
    save_mnist_bmp(train_x + 784*i, "train_%05d.bmp", i);

    //画像を読み込む
    float *x = load_mnist_bmp(argv[1]);

    //読み込んだ画像で推論
    int  predict = 0;
    predict = inference6(A1, A2, A3, b1, b2, b3, x,y);

    printf("読み込んだ画像は%dです。\n",predict);
    return 0;
}
