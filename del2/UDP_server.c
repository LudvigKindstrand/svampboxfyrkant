#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>

#define PORT 8037
#define EPOCH_OFFSET 2208988800L

int main() {
    // Create socket
    int server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    while (1) {
        time_t current_time;
        uint32_t net_time;
        struct sockaddr_in client_addr;
        socklen_t client_addrlen = sizeof(client_addr);

        // Receive request from client
        ssize_t bytes_received = recvfrom(server_socket, NULL, 0, 0, (struct sockaddr *)&client_addr, &client_addrlen);
        if (bytes_received < 0) {
            perror("Error receiving data");
            continue;
        }

        // Get current time
        time(&current_time);
        net_time = htonl((uint32_t)(current_time + EPOCH_OFFSET));

        // Send time to client
        sendto(server_socket, (const char *)&net_time, sizeof(net_time), 0, (const struct sockaddr *)&client_addr, client_addrlen);
    }

    // Close socket
    close(server_socket);

    return 0;
}
