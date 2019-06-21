# cpro2018
このリポジトリは電電プログラミング演習の手助けになることを目的として作られました。

## Getting Started
1. レポジトリをクローンする
```bash
git clone https://github.com/naoki-kishi/cpro2018.git
```

2. 6層NNの学習用のコードをビルドする。以下のコマンドを実行することで`learn_6layers`という実行ファイルが生成される。(エラーが出るが気にしない。)
```bash
cd final
gcc -g -Wall -O2 -o learn_6layers learn_6layers.c nn_module.c  -lm
```

3. 学習を行う。3つの層の学習データを保存するファイル名を引数として渡す。
```bash
./learn_6layers save1.dat save2.dat save3.dat

-----------------------------------------------------
Epoch 1/10
Accuracy : 72.315002 ％ 
Loss  Average: 0.897382
Accuracy Val : 73.300003 ％ 
Loss  Average Val : 0.869351
-----------------------------------------------------
Epoch 2/10
Accuracy : 81.669998 ％ 
Loss  Average: 0.596373
Accuracy Val : 82.540001 ％ 
Loss  Average Val : 0.567866
-----------------------------------------------------
Epoch 3/10
Accuracy : 85.968330 ％ 
Loss  Average: 0.477644
Accuracy Val : 86.739998 ％ 
Loss  Average Val : 0.452612
-----------------------------------------------------
Epoch 4/10
Accuracy : 87.873337 ％ 
Loss  Average: 0.415646
Accuracy Val : 88.360001 ％ 
Loss  Average Val : 0.397430
-----------------------------------------------------
Epoch 5/10
Accuracy : 89.021667 ％ 
Loss  Average: 0.376220
Accuracy Val : 89.489998 ％ 
Loss  Average Val : 0.359114
-----------------------------------------------------
Epoch 6/10
Accuracy : 89.830002 ％ 
Loss  Average: 0.348752
Accuracy Val : 90.459999 ％ 
Loss  Average Val : 0.332884
-----------------------------------------------------
Epoch 7/10
Accuracy : 90.298332 ％ 
Loss  Average: 0.330014
Accuracy Val : 90.900002 ％ 
Loss  Average Val : 0.315254
-----------------------------------------------------
Epoch 8/10
Accuracy : 90.855003 ％ 
Loss  Average: 0.313398
Accuracy Val : 91.180000 ％ 
Loss  Average Val : 0.300171
-----------------------------------------------------
Epoch 9/10
Accuracy : 91.180000 ％ 
Loss  Average: 0.301298
Accuracy Val : 91.489998 ％ 
Loss  Average Val : 0.288402
-----------------------------------------------------
Epoch 10/10
Accuracy : 91.518333 ％ 
Loss  Average: 0.289385
Accuracy Val : 91.809998 ％ 
Loss  Average Val : 0.278479
```

4. 推論用のコードをビルドする。
```bash
gcc -g -Wall -O2 -o inference_from_image inference_from_image.c nn_module.c  -lm
```

5. 推論を行う。
```bash
/inference_from_image save1.dat save2.dat save3.dat "./default_font/2.bmp"

読み込んだ画像は2です。
```
