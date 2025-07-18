#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t n;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%d\n", SERVER_IP, SERVER_PORT);

    // Send message to server
    printf("Enter message: ");
    if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
        perror("fgets");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Receive response from server
    n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (n < 0) {
        perror("recv");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';
    printf("Server response: %s\n", buffer);

    close(sockfd);
    return 0;
}
