//サーバへ文字列を送信するクライアントプログラム
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "kadai_method.h"

#define BUFSIZE  256

void chop(char *str)
{
    char *p = strchr(str, '\n');
    if (p != NULL)
        *p = '\0';
}


int main(int argc, char *argv[])
{
    int socket_fd; // socket() の返すファイル識別子
    struct sockaddr_in server; //サーバプロセスのソケットアドレス情報
    struct hostent *hp; //ホスト情報
    uint16_t port; //ポート番号
    char buffer[BUFSIZE]; //メッセージを格納するバッファ
    

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[2]);

    //ソケットの作成 : INET ドメイン・ストリーム型
    socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("client: socket");
        exit(EXIT_FAILURE);
    }

    //サーバプロセスのソケットアドレス情報の設定
    memset((void *) &server, 0, sizeof(server)); //アドレス情報構造体の初期化
    server.sin_family = PF_INET; //プロトコルファミリの設定
    server.sin_port = htons(port); //ポート番号の設定

    // argv[1] のマシンのIPアドレスを返す
    if ((hp = gethostbyname(argv[1])) == NULL) {
        perror("client: gethostbyname");
        exit(EXIT_FAILURE);
    }

    close(socket_fd);




    pthread_t th1,th2;
    void *rval;

    srand((unsigned int)time(NULL));

    //スレッドth1を生成し、recvText("th1")を呼び出す
    if(pthread_create(&th1, NULL, recvText, (void*)"th1") != 0){
        perror("Thread creation failed.\n");
        exit(EXIT_FLURE);
    }

    //スレッドth2を生成し、sendText("th2")を呼び出す
    if(pthread_create(&th2, NULL, sendText, (void*)"th2") != 0){
        perror("Thread creation failed.\n");
        exit(EXIT_FLURE);
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
    printf("the process joins with thread th1\n");
    if(pthread_join(th2, &rval) != 0){
        perror("Failed to join with th1.\n");
    }else{
        printf("finished th2 (thread ID = %p)\n", (void*)*(pthread_t*)rval);
        free(rval);
    }


    return 0;
}

    

void recvText(void *arg){
    char buffer[BUFSIZE]; //メッセージを格納するバッファ
    
    while (1) {
        // 送信データを読み込む
        memset(buffer, '\0', BUFSIZE);
        // データ受信
        recv(socket, buffer, BUFSIZE,0);
        printf("from server: %s\n", buffer);
        if (strcmp(buffer, "quit") == 0)
            break;
    }
}

void sendText(void *arg){
    char buffer[BUFSIZE]; //メッセージを格納するバッファ

    while (1) {
        // 送信データを読み込む
        memset(buffer, '\0', BUFSIZE);
        printf(">");
        if (fgets(buffer, BUFSIZE, stdin) == NULL)
            strcpy(buffer, "quit");
        chop(buffer);
        //データ送信
        send(socket, buffer, BUFSIZE,0);
        if (strcmp(buffer, "quit") == 0)
            break;
    }
}