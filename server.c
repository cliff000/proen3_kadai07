//クライアントから文字列を受信するサーバプログラム
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define BUFSIZE  256
#include <time.h>
#include <pthread.h>

void *recvText(void *arg);
void *sendText(void *arg);
void *recvUserName(void *arg);
void *sendUserName(void *arg);

void chop(char *str)
{
    char *p = strchr(str, '\n');
    if (p != NULL)
        *p = '\0';
}


int connected_socket; // accept()が返すファイル識別子
int exitFlag = 0; //終了フラグ
char myName[BUFSIZE]; //自分の名前
char opponentName[BUFSIZE]; //相手の名前

int main(int argc, char *argv[])
{
    int listening_socket; // socket()が返すファイル識別子
    struct sockaddr_in server; //サーバプロセスのソケットアドレス情報
    struct sockaddr_in client; //クライアントプロセスのソケットアドレス情報
    socklen_t fromlen; //クライアントプロセスのソケットアドレス情報の長さ
    uint16_t port; //ポート番号
    
    int temp = 1;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]); 

    //ソケットの作成: INET ドメイン・ストリーム型
    listening_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (listening_socket == -1) {
        perror("server: socket");
    exit(EXIT_FAILURE);
    }

    //ソケットオプションの設定
    // SO_REUSEADDR を指定しておかないと、サーバの異常終了後に再起動した場合
    //数分間ポートがロックされ bind() に失敗することがある
    if (setsockopt(listening_socket, SOL_SOCKET,
            SO_REUSEADDR, (void *) &temp, sizeof(temp))) {
        perror("server: setsockopt");
        exit(EXIT_FAILURE);
    }

    // サーバプロセスのソケットアドレス情報の設定
    memset((void *) &server, 0, sizeof(server));
    //アドレス情報構造体の初期化
    server.sin_family = PF_INET;
    //プロトコルファミリの設定
    server.sin_port = htons(port);
    //ポート番号の設定
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    //ソケットにアドレスをバインド
    if (bind(listening_socket, (struct sockaddr *) &server,
        sizeof(server)) == -1) {
        perror("server: bind");
        exit(EXIT_FAILURE);
    }

    //接続要求の受け入れ準備
    //バインドされたソケットを待機状態に
    if (listen(listening_socket, 5) == -1) {
        perror("server: listen");
        exit(EXIT_FAILURE);
    }

    memset((void *) &client, 0, sizeof(client));
    fromlen = sizeof(client);
    //クライアントからの接続要求を受け入れ、通信経路を確保する
    //クライアントと接続したソケットの識別子が connected_socket に格納される
    connected_socket = accept(listening_socket,
                (struct sockaddr *) &client, &fromlen);

    if (connected_socket == -1) {
        perror("server: accept");
        exit(1);
    }

    // listening_socket は必要なくなったので閉じる
    close(listening_socket);



    //ユーザー名送受信
    pthread_t recvName,sendName;
    void *rval;

    srand((unsigned int)time(NULL));

    //スレッドrecvNameを生成し、recvUserName("recvName")を呼び出す
    if(pthread_create(&recvName, NULL, recvUserName, (void *) "recvName") != 0){
        perror("Thread creation failed.\n");
        exit(EXIT_FAILURE);
    }
    //スレッドsendNameを生成し、sendUserName("sendName")を呼び出す
    if(pthread_create(&sendName, NULL, sendUserName, (void *) "sendName") != 0){
        perror("Thread creation failed.\n");
        exit(EXIT_FAILURE);
    }

    //スレッドrecvNameの終了を待つ
    if(pthread_join(recvName, &rval) != 0){
        perror("Failed to join with recvName.\n");
    }else{
        free(rval);
    }
    //スレッドsendNameの終了を待つ
    if(pthread_join(sendName, &rval) != 0){
        perror("Failed to join with sendName.\n");
    }else{
        free(rval);
    }



    printf("--------------- chat start ---------------\n");



    // スレッド作成
    pthread_t recv,send;


    srand((unsigned int)time(NULL));

    //スレッドrecvを生成し、recvText("recv")を呼び出す
    if(pthread_create(&recv, NULL, recvText, (void *) "recv") != 0){
        perror("Thread creation failed.\n");
        exit(EXIT_FAILURE);
    }

    //スレッドsendを生成し、sendText("send")を呼び出す
    if(pthread_create(&send, NULL, sendText, (void *) "send") != 0){
        perror("Thread creation failed.\n");
        exit(EXIT_FAILURE);
    }


    //送受信のスレッドが終わるまで待つ
    while(exitFlag == 0){
        if(pthread_tryjoin_np(recv, &rval) == 0)
            exitFlag = 1;
        if(pthread_tryjoin_np(send, &rval) == 0)
            exitFlag = 1;
    }
    free(rval);
    printf("\n---------------  finish  ---------------\n");


    return 0;
}


//ユーザー名受信を行う関数
void *recvUserName(void *arg){
    pthread_t *thread_id = (pthread_t *) malloc(sizeof(pthread_t));
    *thread_id = pthread_self(); // 自分のスレッド番号を得る

    memset(opponentName, '\0', BUFSIZE);
    recv(connected_socket, opponentName, BUFSIZE,0);

    pthread_exit((void *) thread_id);
}
//ユーザー名送信を行う関数
void *sendUserName(void *arg){
    pthread_t *thread_id = (pthread_t *) malloc(sizeof(pthread_t));
    *thread_id = pthread_self(); // 自分のスレッド番号を得る

    memset(myName, '\0', BUFSIZE);
    printf("Please tell me your name\n>");
    if (fgets(myName, BUFSIZE, stdin) == NULL)
        strcpy(myName, "cliant");
    chop(myName);
    //データ送信
    send(connected_socket, myName, BUFSIZE,0);

    pthread_exit((void *) thread_id);
}


// 受信を行う関数
void *recvText(void *arg){
    char buffer[BUFSIZE]; //メッセージを格納するバッファ

    pthread_t *thread_id = (pthread_t *) malloc(sizeof(pthread_t));
    *thread_id = pthread_self(); // 自分のスレッド番号を得る
    
    while (1) {
        // 受信データを読み込む
        memset(buffer, '\0', BUFSIZE);
        // データ受信
        recv(connected_socket, buffer, BUFSIZE,0);
        printf("\nfrom %s: %s\n", opponentName, buffer);
        printf("%s>", myName);
        fflush(stdout);
        if (strcmp(buffer, "quit") == 0)
            break;

        if(exitFlag)
            break;
    }

    pthread_exit((void *) thread_id);
}

// 送信を行う関数
void *sendText(void *arg){
    char buffer[BUFSIZE]; //メッセージを格納するバッファ

    pthread_t *thread_id = (pthread_t *) malloc(sizeof(pthread_t));
    *thread_id = pthread_self(); // 自分のスレッド番号を得る

    while (1) {
        // 送信データを読み込む
        memset(buffer, '\0', BUFSIZE);
        printf("%s>", myName);
        if (fgets(buffer, BUFSIZE, stdin) == NULL)
            strcpy(buffer, "quit");
        chop(buffer);
        //データ送信
        send(connected_socket, buffer, BUFSIZE,0);
        if (strcmp(buffer, "quit") == 0)
            break;

        if(exitFlag)
            break;
    }

    pthread_exit((void *) thread_id);
}
