# cpro2018
このGitHubは電電プログラミング演習の手助けになることを目的として作られました。
コピペはしないでください

## Getting Started
1. レポジトリをクローンする
```bash
git clone https://github.com/naoki-kishi/cpro2018.git
```

2. 6層NNの学習用のコードをビルドする。以下のコマンドを実行することで`learn_6layers`という実行ファイルが生成される。
```bash
cd final
gcc -g -Wall -O2 -o learn_6layers learn_6layers.c nn_module.c  -lm
```

3. 学習を行う。3つの層の学習データを保存するファイル名を引数として渡す。
```bash
./learn_6layers save1.dat save2.dat save3.dat
```

4. 推論を行う。
