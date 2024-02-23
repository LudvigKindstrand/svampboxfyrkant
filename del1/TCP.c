#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 

#define SERVER_PORT 8080
#define BUFFER_SIZE 4096

// Funktion för att skicka felmeddelande 404 Not Found
void error(int client_sd) {
    const char *response_404 = "HTTP/1.1 404 Not Found\r\nServer: Demo Web Server\r\n\r\nFile not found";
    write(client_sd, response_404, strlen(response_404)); // Skicka 404 HTTP-svar till klienten
}

// Funktion för att skicka Content-Type i HTTP-header baserat på filtyp
void SendContentType(const char *type, int socket) {
    char response[BUFFER_SIZE];
    if (strcmp(type, "jpg") == 0) {
        snprintf(response, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nServer: Demo Web Server\r\nContent-Type: image/jpeg\r\n\r\n");
    } else if (strcmp(type, "html") == 0) {
        snprintf(response, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nServer: Demo Web Server\r\nContent-Type: text/html\r\n\r\n");
    } else {
        // Okänd filtyp, skicka 404 header
        snprintf(response, BUFFER_SIZE, "HTTP/1.1 404 Not Found\r\nServer: Demo Web Server\r\n\r\n");
    }

    write(socket, response, strlen(response)); // Skicka HTTP-header till klienten
}

// Funktion för att behandla HTTP-förfrågning och skicka svar till klienten
void process_http_request(int client_sd, const char *filename, const char *content_type) {
    FILE *file = fopen(filename, "rb");

    if (file != NULL) {
        // Skicka HTTP-header med lämplig Content-Type
        SendContentType(content_type, client_sd);

        // Skicka innehållet i filen
        char buffer[BUFFER_SIZE];
        size_t bytes_read;

        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            write(client_sd, buffer, bytes_read); // Skicka filens innehåll till klienten
        }

        // Stäng filen
        fclose(file);
    } else {
        // Filen hittades inte (404)
        error(client_sd);
    }
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    int server_socket, client_socket;
    socklen_t client_addrlen = sizeof(client_addr);

    // Skapa en socket för servern
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Konfigurera serveradressen och porten
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind servern till den angivna porten
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    // Lyssna efter inkommande anslutningar
    if (listen(server_socket, SOMAXCONN) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Acceptera inkommande anslutning från klient
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addrlen);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Anslutning accepterad\n");

        // Läs inkommande data från klienten till en buffert
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(client_socket, buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            perror("Fel vid läsning från socket");
            close(client_socket);
            continue;
        } else if (bytes_read == 0) {
            // Ingen data läst, anslutningen är stängd
            close(client_socket);
            continue;
        }

        // Noll-terminera bufferten för att behandla den som en sträng
        buffer[bytes_read] = '\0';

        // Parsa sökvägen till filen från HTTP-förfrågan
        char *token = strtok(buffer, " ");
        token = strtok(NULL, " ");
        if (token) {
            // Ta bort första "/" från sökvägen
            memmove(token, token + 1, strlen(token));

            char temp[BUFFER_SIZE];
            strcpy(temp, token);

            // Parsa ut Content-Type
            char *content_type = "text/html"; // Standard Content-Type
            char *ContentToken = strtok(temp, ".");
            ContentToken = strtok(NULL, " ");
            if (ContentToken) {
                process_http_request(client_socket, token, ContentToken); // Hantera HTTP-förfrågan och skicka svar till klienten
            } else {
                // Fel vid parsning av Content-Type, skicka 404
                error(client_socket);
            }
        }

        // Stäng klientens socket
        close(client_socket);
        printf("Anslutning stängd\n");
    }

    // Stäng serverns socket
    close(server_socket);

    return 0;
}
