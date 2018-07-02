//プロトタイプ宣言
void print(int m,int n,const float *x);  //m*n行列xを表示
void copy(int m, int n, const float *x, float *y);  //m*n行列xをyにコピーする
void fc(int m, int n, const float *x, const float *A, const float *b, float *y);  //m*n行列Aを用いてy = A*x +b を計算する
void relu(int n,const float *x, float *y);  //m列ベクトルのxに対して、relu演算を行う y = relu(x)
void prelu(int n,const float *x, float *y,float a);  //m列ベクトルのxに対して、prelu演算を行う y = prelu(x)
void softmax(int n,const float *x, float *y);  //m行のベクトルxに対して、y = softmax(x)
int inference3(const float *A, const float *b, const float *x,float * y);  //3層による推論を行い、得られた結果[0:9]を返す
int inference6(const float *A1,const float *A2,const float *A3, const float *b1,  const float *b2, const float *b3, const float *x,float * y);  //6層による推論を行い、得られた結果[0:9]を返す
void softmaxwithloss_bwd(int n, const float *y, unsigned char t, float *dEdx);  //softmaxの逆誤差伝播を行う
void relu_bwd(int n, const float * x, const float * dEdy, float * dEdx);  //reluの逆誤差伝播を行う
void prelu_bwd(int n, const float * x, const float * dEdy, float * dEdx,const float a, float dEda);  //preluの逆誤差伝播を行う
void fc_bwd(int m, int n, const float * x, const float * dEdy, const float * A, float * dEdA, float * dEdb, float * dEdx);  //FC層の逆誤差伝播を行う
void backward3(const float *A, const float *b, const float *x, unsigned char t, float *y, float *dEdA, float *dEdb);  //3層の逆誤差伝播を行う
void backward6(const float *A1,const float *A2,const float *A3, const float *b1,  const float *b2, const float *b3, const float *x, unsigned char t, float *y,
    float *dEdA1,float *dEdA2,float *dEdA3, float *dEdb1,float *dEdb2,float *dEdb3);  //6層の逆誤差伝播を行う
void shuffle(int n, int *x);  //n行ベクトルxの配列をシャッフルする
float cross_entropy_error(const float * y, int t);  //損失関数を求める
void add(int n, const float *x, float *o);  //n行ベクトルxをoに足す
void scale(int n, float x, float *o);  //n行ベクトルoにxをかける
void init(int n, float x, float *o);  //n行ベクトルoをxで全要素を初期化
void rand_init(int n, float *o);  //[-1:1]の範囲でn行ベクトルoを初期化
float uniform( void );//一様乱数を生成
void rand_init_by_normal_dist(int n, float *o,float mu,float sigma);//n行ベクトルを正規分布で初期化
void save(const char * filename, int m, int n, const float * A, const float * b);  //一層分のパラメータを保存
void load(const char * filename, int m, int n, float * A, float * b);  //一層分のパラメータを読み込む
