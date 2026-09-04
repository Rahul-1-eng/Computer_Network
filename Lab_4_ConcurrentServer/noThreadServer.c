// noThreadServer.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int tcpSock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in tcpAddr;
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_addr.s_addr = INADDR_ANY;
    tcpAddr.sin_port = htons(8080);
    
    bind(tcpSock, (struct sockaddr*)&tcpAddr, sizeof(tcpAddr));
    
    // Exactly 2 spots in the waiting room
    listen(tcpSock, 2); 
    
    while(1) {
        struct sockaddr_in cliAddr;
        socklen_t cliLen = sizeof(cliAddr);
        
        printf("Waiting for someone to connect...\n");
        int newSock = accept(tcpSock, (struct sockaddr*)&cliAddr, &cliLen);
        printf("Client connected: %s:%d\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));
        
        char netBuf[256];
        // The server gets TRAPPED in this loop with Client 1
        while(1) {
            int bytes = recv(newSock, netBuf, 255, 0);
            if (bytes <= 0) break;
            netBuf[bytes] = '\0';
            netBuf[strcspn(netBuf, "\r\n")] = 0;
            
            if (strcmp(netBuf, "EXIT") == 0) break;
            
            char respBuf[] = "I am talking to you!\n";
            send(newSock, respBuf, strlen(respBuf), 0);
        }
        
        close(newSock);
        printf("Client left. Checking the waiting room...\n");
    }
    return 0;
}
