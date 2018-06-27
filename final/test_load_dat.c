#include "nn.h"
#include "util.h"
#include <time.h>

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

    //テストデータでの推論
    int i;
    int sum = 0;
    float loss_sum = 0;
    float acc= 0.0;

    for(i=0 ; i<test_count ; i++) {
        if(inference6(A1, A2, A3, b1, b2, b3, test_x + i*784,y) == test_y[i]) {
            sum++;
        }
        loss_sum += cross_entropy_error(y,test_y[i]);

    }
    acc = sum * 100.0 / test_count;

    printf("Accuracy : %f ％ \n",acc);
    printf("Loss Average : %f\n",loss_sum/test_count);

    return 0;
}
