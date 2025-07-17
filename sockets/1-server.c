#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <asm-generic/socket.h>

#define PORT 12345

int setup_server_socket(void);

/**
* main - Opens a socket and listens on a port 12345
* Accepts a client connection
* Return: 0 on success, 1 on failure if binding fails
*/
int main(void)
{
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	int client_fd;

	int server_fd = setup_server_socket();

	if (server_fd == -1)
	{
		return (1);
	}

	client_fd = accept(server_fd,
	(struct sockaddr *)&client_addr, &client_addr_len);

	if (client_fd == -1)
	{
		perror("accept failed");
		close(server_fd);
		return (1);
	}

	printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

	close(client_fd);
	close(server_fd);

	return (0);
}

/**
* setup_server_socket - Set up a server socket and start listening
*
* This function creates a server socket, binds it to the specified port,
* and starts listening for incoming connections.
*
* Return: The file descriptor of the server, or -1 on failure.
*/
int setup_server_socket(void)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	int enable = 1;

	if (server_fd == -1)
	{
		perror("socket failed");
		return (-1);
	}


	if (setsockopt(server_fd, SOL_SOCKET,
	SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(int)) < 0) {
		perror("SO_REUSEADDR | SO_REUSEPORT failed");
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		perror("bind failed");
		close(server_fd);
		return (-1);
	}

	if (listen(server_fd, 10) == -1)
	{
		perror("listen failed");
		close(server_fd);
		return (-1);
	}

	return (server_fd);
}
