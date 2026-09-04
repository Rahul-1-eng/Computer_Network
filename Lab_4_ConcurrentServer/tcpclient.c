//tcpclient.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int cliSock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in srvAddr;
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &srvAddr.sin_addr);
    connect(cliSock, (struct sockaddr*)&srvAddr, sizeof(srvAddr));

    char hostName[50], recType[10], netBuf[256];
    while(1) {
        printf("Enter domain (or EXIT): ");
        scanf("%s", hostName);
        if(strcmp(hostName, "EXIT") == 0) {
            send(cliSock, "EXIT", 4, 0);
            break;
        }
        printf("Enter type: ");
        scanf("%s", recType);
        
        sprintf(netBuf, "%s %s", hostName, recType);
        send(cliSock, netBuf, strlen(netBuf), 0);
        
        memset(netBuf, 0, sizeof(netBuf));
        recv(cliSock, netBuf, 255, 0);
        printf("Server response:\n%s", netBuf);
    }
    close(cliSock);
    return 0;
}
