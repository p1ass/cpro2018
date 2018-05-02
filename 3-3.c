/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GU 0
#define CHOKI 2
#define PA 5

int hands[3] = {GU,CHOKI,PA};

 //自分の手を入力する
int input_hand(void){
    int hand = GU;
    while(1){
        printf("Your input(0,2,5):");
        scanf("%d",&hand);

        int i;
        for(i=0;i<3;i++){
            if(hand == hands[i]){
                return hand;
            }
        }
        printf("Invalid input => Input again.\n");
    }
    return hand;
}

//ランダムに手を決める
int get_ran_hand(void){
    int hand = GU;

    srand(time(NULL));
    hand = hands[rand() % 3];
    return hand;
}

int main(void){
    int my_hand;
    int cpu_hand;

    int judge[3][2] = {
                {GU,CHOKI},
                {CHOKI,PA},
                {PA,GU}};

    //メインループ
    while(1){
        //手を決める
        my_hand = input_hand();
        cpu_hand = get_ran_hand();

        //お互いの手を表示
        printf("Comp:%d vs You:%d => ",cpu_hand,my_hand);

        //勝敗判定
        int i;
        for(i=0;i<3;i++){
            if(cpu_hand == judge[i][0] && my_hand == judge[i][1]){
                printf("Comp win.\n");
                return 0;
            }else if(my_hand == judge[i][0] && cpu_hand == judge[i][1]){
                printf("You win.\n");
                return 0;
            }
        }
        printf("Try again.\n");
    }
    return 0;
}
