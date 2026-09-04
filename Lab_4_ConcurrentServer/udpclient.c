//udpclient.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int cliSock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in srvAddr;
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(8081);
    inet_pton(AF_INET, "127.0.0.1", &srvAddr.sin_addr);

    char netBuf[256];
    sendto(cliSock, "STATUS", 6, 0, (struct sockaddr*)&srvAddr, sizeof(srvAddr));
    
    socklen_t addrLen = sizeof(srvAddr);
    int bytes = recvfrom(cliSock, netBuf, 255, 0, (struct sockaddr*)&srvAddr, &addrLen);
    netBuf[bytes] = '\0';
    
    printf("Response:\n%s", netBuf);
    close(cliSock);
    return 0;
}
