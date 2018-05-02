/*
Author : Naoki Kishi (KUEE 2T13)
GitHub URL : https://github.com/naoki-kishi/cpro2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

 //自分の手を入力する
int input_hand(void){
    int hand = 0;
    while(1){
        printf("Your input(0,2,5):");
        scanf("%d",&hand);

        if(hand != 0 && hand != 2 && hand != 5){
            printf("Invalid input => Input again.\n");
        }
        else{
            break;
        }
    }
    return hand;
}

//ランダムに手を決める
int get_ran_hand(void){
    int hand = 0;
    srand(time(NULL));
    hand = rand() % 3;

    if (hand == 1){
        hand = 2;
    }else if(hand == 2){
        hand = 5;
    }

    return hand;
}

int main(void){
    int my_hand = 0;
    int cpu_hand = 0;

    int judge[3][2] = {
                {0,2},
                {2,5},
                {5,0}};

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
                printf("Comp win.");
                return 0;
            }else if(my_hand == judge[i][0] && cpu_hand == judge[i][1]){
                printf("You win.");
                return 0;
            }
        }
        printf("Try again.\n");
    }
    return 0;
}
