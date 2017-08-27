/*************************************************************************
    > File Name: server.cpp
    > Author: Ukey
    > Mail: gsl110809@gmail.com
    > Created Time: 2017年02月11日 星期六 20时08分39秒
 ************************************************************************/
#include "utility.h"

int main(int argc, char *argv[])
{
	
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);

	int listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0)
	{
		perror("listener");
		exit(-1);
	}
	printf("listen socket created\n");
	
	if(bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr)))
	{
		perror("bind error");
		exit(-1);
	}

	int ret = listen(listener, 20);
	if(ret < 0)
	{
		perror("listen error");
		exit(-1);
	}
	printf("Start to listen: %s\n", SERVER_IP);

	int epfd = epoll_create(EPOLL_SIZE);
	if(epfd < 0)
	{
		perror("epfd error");
		exit(-1);
	}
	printf("epoll created, epollfd = %d\n", epfd);
	static struct epoll_event events[EPOLL_SIZE];

	addfd(epfd, listener, true);

	while(1)
	{
		int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
		if(epoll_events_count < 0)
		{
			perror("epoll failure");
			break;
		}

		printf("epoll_events_count = %d\n", epoll_events_count);

		for(int i = 0; i < epoll_events_count; i++)
		{
			int sockfd = events[i].data.fd;

			if(sockfd == listener)
			{
				struct sockaddr_in client_address;
				socklen_t client_addrlength = sizeof(struct sockaddr_in);
				int clientfd = accept(listener, (struct sockaddr *)&client_address, &client_addrlength);
				printf("client connection from: %s : %d(IP : port), clientfd = %d \n",
				inet_ntoa(client_address.sin_addr),
				ntohs(client_address.sin_port),
				clientfd);

				addfd(epfd, clientfd, true);

				clients_list.push_back(clientfd);
				printf("Add new clientfd = %d to epoll\n", clientfd);
				printf("Now there are %d clients in the chat room\n", (int)clients_list.size());

				printf("welcome message\n");
				char message[BUF_SIZE];
				bzero(message, BUF_SIZE);
				sprintf(message, SERVER_WELCOME, clientfd);
				int ret = send(clientfd, message, BUF_SIZE, 0);
				if(ret < 0)
				{
					perror("send error");
					exit(-1);
				}
			}
			else
			{
				int ret = sendBroadcastmessage(sockfd);
				if(ret < 0)
				{
					perror("error");
					exit(-1);
				}
			}
		}
	}
	close(listener);
	close(epfd);
}

