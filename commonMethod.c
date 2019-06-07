#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "kadai_method.h"

void recvText(int soket){
    while (1) {
        // 送信データを読み込む
        memset(buffer, '\0', BUFSIZE);
        // データ受信
        recv(connected_socket, buffer, BUFSIZE,0);
        printf("from client: %s\n", buffer);
        if (strcmp(buffer, "quit") == 0)
            break;
    }
}

void sendText(int soket){
    while (1) {
        if (fgets(buffer, BUFSIZE, stdin) == NULL)
            strcpy(buffer, "quit");
        chop(buffer);
        //データ送信
        send(connected_socket, buffer, BUFSIZE,0);
        if (strcmp(buffer, "quit") == 0)
            break;
    }
}