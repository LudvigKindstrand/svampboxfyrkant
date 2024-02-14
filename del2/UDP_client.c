#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 37

int main() {
    // Create socket
    int client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Assuming server runs on localhost

    // Send request to server
    sendto(client_socket, NULL, 0, 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));

    // Receive time from server
    time_t server_time;
    struct sockaddr_in from_addr;
    socklen_t from_addrlen = sizeof(from_addr);
    ssize_t bytes_received = recvfrom(client_socket, (char *)&server_time, sizeof(server_time), 0, (struct sockaddr *)&from_addr, &from_addrlen);
    if (bytes_received < 0) {
        perror("Error receiving data");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Convert time to a readable format
    printf("Time from server: %s", ctime(&server_time));

    // Close socket
    close(client_socket);

    return 0;
}
