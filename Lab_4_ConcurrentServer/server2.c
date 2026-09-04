// server2.c
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

// NEW: Global tracker and mutex
int activeClients = 0;
pthread_mutex_t mutLock = PTHREAD_MUTEX_INITIALIZER;

// NEW: UDP Thread function
void* udpWorker(void* arg) {
    int udpSock = *(int*)arg;
    char netBuf[256];
    struct sockaddr_in cliAddr;
    socklen_t addrLen = sizeof(cliAddr);
    
    while(1) {
        int bytes = recvfrom(udpSock, netBuf, 255, 0, (struct sockaddr*)&cliAddr, &addrLen);
        if (bytes > 0) {
            netBuf[bytes] = '\0';
            if (strncmp(netBuf, "STATUS", 6) == 0) {
                char respBuf[256];
                pthread_mutex_lock(&mutLock);
                int currCount = activeClients;
                pthread_mutex_unlock(&mutLock);
                sprintf(respBuf, "Server active. Connected clients: %d\n", currCount);
                sendto(udpSock, respBuf, strlen(respBuf), 0, (struct sockaddr*)&cliAddr, addrLen);
            }
        }
    }
    return NULL;
}

void* tcpWorker(void* arg) {
    int cliSock = *(int*)arg;
    free(arg);
    char netBuf[256];
    
    // NEW: Increment count safely
    pthread_mutex_lock(&mutLock);
    activeClients++;
    pthread_mutex_unlock(&mutLock);
    
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
    
    // NEW: Decrement count safely
    pthread_mutex_lock(&mutLock);
    activeClients--;
    pthread_mutex_unlock(&mutLock);
    
    close(cliSock);
    return NULL;
}

int main() {
    int tcpSock = socket(AF_INET, SOCK_STREAM, 0);
    int udpSock = socket(AF_INET, SOCK_DGRAM, 0); // NEW
    
    struct sockaddr_in tcpAddr, udpAddr;
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_addr.s_addr = INADDR_ANY;
    tcpAddr.sin_port = htons(8080);
    
    udpAddr = tcpAddr;
    udpAddr.sin_port = htons(8081); // NEW: Different port for UDP
    
    bind(tcpSock, (struct sockaddr*)&tcpAddr, sizeof(tcpAddr));
    bind(udpSock, (struct sockaddr*)&udpAddr, sizeof(udpAddr)); // NEW
    
    listen(tcpSock, 5);
    
    // NEW: Start background UDP thread
    pthread_t udpThrd;
    pthread_create(&udpThrd, NULL, udpWorker, &udpSock);
    
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
