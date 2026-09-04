// server1.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef struct {
    char hostName[50];
    char recType[10];
    char recVal[50];
} DnsRec;

DnsRec db[] = {
    {"www.example.com", "A", "192.168.1.10"},
    {"mail.example.com", "A", "192.168.1.20"},
    {"web.example.com", "CNAME", "www.example.com"},
    {"example.com", "MX", "mail.example.com"},
    {"example.com", "NS", "ns1.example.com"}
};

void* tcpWorker(void* arg) {
    int cliSock = *(int*)arg;
    free(arg);
    char netBuf[256];
    
    while(1) {
        int bytes = recv(cliSock, netBuf, 255, 0);
        if (bytes <= 0) break;
        netBuf[bytes] = '\0';
        netBuf[strcspn(netBuf, "\r\n")] = 0;
        
        if (strcmp(netBuf, "EXIT") == 0) break;
        
        char hostName[50] = {0}, recType[10] = {0};
        sscanf(netBuf, "%s %s", hostName, recType);
        
        char* ansStr = "Record not found\n";
        for (int i = 0; i < 5; i++) {
            if (strcmp(db[i].hostName, hostName) == 0 && strcmp(db[i].recType, recType) == 0) {
                ansStr = db[i].recVal;
                break;
            }
        }
        char respBuf[256];
        sprintf(respBuf, "%s\n", ansStr);
        send(cliSock, respBuf, strlen(respBuf), 0);
    }
    close(cliSock);
    return NULL;
}

int main() {
    int tcpSock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in tcpAddr;
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_addr.s_addr = INADDR_ANY;
    tcpAddr.sin_port = htons(8080);
    
    bind(tcpSock, (struct sockaddr*)&tcpAddr, sizeof(tcpAddr));
    listen(tcpSock, 5);
    
    while(1) {
        struct sockaddr_in cliAddr;
        socklen_t cliLen = sizeof(cliAddr);
        int* newSock = malloc(sizeof(int));
        *newSock = accept(tcpSock, (struct sockaddr*)&cliAddr, &cliLen);
        
        printf("Client connected: %s:%d\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));
        
        pthread_t tcpThrd;
        pthread_create(&tcpThrd, NULL, tcpWorker, newSock);
        pthread_detach(tcpThrd);
    }
    return 0;
}
