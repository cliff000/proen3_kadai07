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

void chop(char *str)
{
    char *p = strchr(str, '\n');
    if (p != NULL)
        *p = '\0';
}


int connected_socket; // accept()が返すファイル識別子
int exitFlag = 0; //終了フラグ

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



    // スレッド作成
    pthread_t th1,th2;
    void *rval;

    srand((unsigned int)time(NULL));

    //スレッドth1を生成し、recvText("th1")を呼び出す
    if(pthread_create(&th1, NULL, recvText, (void*)"th1") != 0){
        perror("Thread creation failed.\n");
        exit(EXIT_FAILURE);
    }

    //スレッドth2を生成し、sendText("th2")を呼び出す
    if(pthread_create(&th2, NULL, sendText, (void*)"th2") != 0){
        perror("Thread creation failed.\n");
        exit(EXIT_FAILURE);
    }


    //スレッドth1の終了を待つ
    printf("the process joins with thread th1\n");
    if(pthread_join(th1, &rval) != 0){
        perror("Failed to join with th1.\n");
    }else{
        printf("finished th1 (thread ID = %p)\n", (void*)*(pthread_t*)rval);
        free(rval);
    }

    //スレッドth2の終了を待つ
    printf("the process joins with thread th2\n");
    if(pthread_join(th2, &rval) != 0){
        perror("Failed to join with th2.\n");
    }else{
        printf("finished th2 (thread ID = %p)\n", (void*)*(pthread_t*)rval);
        free(rval);
    }

    return 0;
}



void *recvText(void *arg){
    char buffer[BUFSIZE]; //メッセージを格納するバッファ
    
    while (1) {
        // 受信データを読み込む
        memset(buffer, '\0', BUFSIZE);
        // データ受信
        recv(connected_socket, buffer, BUFSIZE,0);
        printf("from client: %s\n", buffer);
        if (strcmp(buffer, "quit") == 0)
            exitFlag = 1;

        if(exitFlag)
            break;
    }
}

void *sendText(void *arg){
    char buffer[BUFSIZE]; //メッセージを格納するバッファ

    while (1) {
        // 送信データを読み込む
        memset(buffer, '\0', BUFSIZE);
        printf(">");
        if (fgets(buffer, BUFSIZE, stdin) == NULL && exitFlag)
            strcpy(buffer, "quit");
        chop(buffer);
        //データ送信
        send(connected_socket, buffer, BUFSIZE,0);
        if (strcmp(buffer, "quit") == 0)
            exitFlag = 1;

        if(exitFlag)
            break;
    }
}