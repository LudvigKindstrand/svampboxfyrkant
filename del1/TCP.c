#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 4096

void send404(int client_sd);

void send_file(int client_sd, const char *filename, char *content_type)
{
	FILE *file = fopen(filename, "rb");

	if (file != NULL)
	{
		// Send HTTP response header
		dprintf(client_sd, "HTTP/1.1 200 OK\r\nServer: Demo Web Server\r\nContent-Type: %s\r\n\r\n", content_type);

		// Send the content of the file
		char buffer[BUFFER_SIZE];
		size_t bytes_read;
		bytes_read = fread(buffer, 1, sizeof(buffer), file);
		if (bytes_read)
		{
			while (bytes_read > 0)
			{
				write(client_sd, buffer, bytes_read);
				bytes_read = fread(buffer, 1, sizeof(buffer), file);
			}
		}
		else
		{
			send404(client_sd);
		}
	}
	else
	{
		send404(client_sd);
	}

	// Close the file
	fclose(file);
}

void send404(int client_sd)
{
	// File not found (404)
	const char *response_404 = "HTTP/1.1 404 Not Found\r\nServer: Demo Web Server\r\n\r\nFile not found";
	write(client_sd, response_404, strlen(response_404));
}

int main()
{
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	int clientaddrlen;
	int request_sd, sd, fd, s, on = 1;
	int bytes;
	char buf[BUFFER_SIZE];
	char *ContentToken;
	char *token;
	int n;

	// Create socket
	request_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	printf("Servers listening and socket is created. \n");

	// Fills in the address structure
	memset(&serveraddr, 0, sizeof(struct sockaddr_in));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVER_PORT);

	setsockopt(request_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

	if (request_sd < 0)
		perror("socket failed");

	// Bind address to socket
	bind(request_sd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in));
	printf("Servers address binded to the socket. \n");

	// Activate connect request queue
	listen(request_sd, SOMAXCONN);

	while (1)
	{
		// Receive connection
		clientaddrlen = sizeof(struct sockaddr_in);
		sd = accept(request_sd, (struct sockaddr *)&clientaddr, &clientaddrlen);

		// Read data from socket and write it
		read(sd, buf, BUFFER_SIZE);

		// Parse out path
		token = strtok(buf, " ");
		token = strtok(NULL, " ");
		if (token)
		{
			// removes first "/" from path
			memmove(token, token + 1, strlen(token));

			char temp[BUFFER_SIZE];
			strcpy(temp, token);

			// Parse out the content type

			ContentToken = strtok(temp, ".");
			ContentToken = strtok(NULL, " ");
			if (ContentToken)
			{
				send_file(sd, token, ContentToken);
			}
			else
			{
				send404(sd);
			}
		}
		close(sd);
	}

	// Close sockets

	close(request_sd);
}