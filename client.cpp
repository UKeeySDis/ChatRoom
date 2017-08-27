/*************************************************************************
    > File Name: client.cpp
    > Author: Ukey
    > Mail: gsl110809@gmail.com
    > Created Time: 2017年02月11日 星期六 22时08分46秒
 ************************************************************************/
#include "utility.h"

int main(int argc, char *argv[])
{
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("sock error");
		exit(-1);
	}

	if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("connect error");
		exit(-1);
	}

	int pipe_fd[2];
	if(pipe(pipe_fd) < 0)
	{
		perror("pipe error");
		exit(-1);
	}

	int epfd = epoll_create(EPOLL_SIZE);
	if(epfd < 0)
	{
		perror("epfd error");
		exit(-1);
	}
	static struct epoll_event events[2];
	
	addfd(epfd, sock, true);
	addfd(epfd, pipe_fd[0], true);

	bool is_clientwork = true;

	char message[BUF_SIZE];

	int pid = fork();
	if(pid < 0)
	{
		perror("fork error");
		exit(-1);
	}
	else if(pid == 0)
	{
		close(pipe_fd[0]);
		printf("Please input 'exit' to exit the chat room \n");

		while(is_clientwork)
		{
			bzero(&message, BUF_SIZE);
			fgets(message, BUF_SIZE, stdin);

			if(strncasecmp(message, EXIT, strlen(EXIT)) == 0)
			{
				is_clientwork = 0;
			}
			else
			{
				if(write(pipe_fd[1], message, strlen(message) - 1) < 0)
				{
					perror("fork error");
					exit(-1);
				}
			}
		}
	}
	else
	{
		close(pipe_fd[1]);

		while(is_clientwork)
		{
			int epoll_event_count = epoll_wait(epfd, events, 2, -1);
			
			for(int i = 0; i < epoll_event_count; i++)
			{
				bzero(&message, BUF_SIZE);

				if(events[i].data.fd == sock)
				{
					int ret = recv(sock, message, BUF_SIZE, 0);

					if(ret == 0)
					{
						printf("Server closed connection: %d\n", sock);
						close(sock);
						is_clientwork = 0;
					}
					else
						printf("%s\n", message);
				}
				else
				{
					int ret = read(events[i].data.fd, message, BUF_SIZE);

					if(ret == 0)
					{
						is_clientwork = 0;
					}
					else
					{
						send(sock, message, BUF_SIZE, 0);
					}
				}
			}
		}
	}

	if(pid)
	{
		close(pipe_fd[0]);
		close(sock);
	}
	else
	{
		close(pipe_fd[1]);
	}
	return 0;

}
