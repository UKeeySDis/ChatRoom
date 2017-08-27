/*************************************************************************
    > File Name: utility.h
    > Author: Ukey
    > Mail: gsl110809@gmail.com
    > Created Time: 2017年02月11日 星期六 17时59分10秒
 ************************************************************************/

#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <iostream>
using namespace std;
#include <list>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clients_list save all the clients's socket
list<int> clients_list;

//server ip
#define SERVER_IP "127.0.0.1"

//server port
#define SERVER_PORT 8888

//epoll size
#define EPOLL_SIZE 5000

//message buffer size
#define BUF_SIZE 0xFFFF

#define SERVER_WELCOME "Welcome you join to the chat room!Your char ID is: Client #%d"

#define SERVER_MESSAGE "ClientID %d say >> %s"

//exit
#define EXIT "EXIT"

#define CAUTION "There is only one in the chat room!"

//set nonblock
int setnonblocking(int sockfd)
{
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
	return 0;
}

//add fd 
void addfd(int epollfd, int fd, bool enable_et)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	if(enable_et)
	{
		ev.events = EPOLLIN | EPOLLET;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	setnonblocking(fd);
	printf("fd added to epoll!\n\n");

}

int sendBroadcastmessage(int clientfd)
{
	//buf[BUF_SIZE] receive new chat message
	//message[BUF_SIZE] save format message
	char buf[BUF_SIZE], message[BUF_SIZE];
	bzero(buf, BUF_SIZE);
	bzero(message, BUF_SIZE);

	//receive message
	printf("read from client(clientID = %d)\n", clientfd);
	int len = recv(clientfd, buf, BUF_SIZE, 0);

	if(len == 0)	//len = 0 means the client closed connection
	{
		close(clientfd);
		clients_list.remove(clientfd);	//server remove the client
		printf("ClientID = %d closed.\nnow there are %d client in the chat room\n", clientfd, (int)clients_list.size());

	}
	else	//broadcast message
	{
		if(clients_list.size() == 1)
		{
			send(clientfd, CAUTION, strlen(CAUTION), 0);
			return len;
		}
		//format message to broadcast
		sprintf(message, SERVER_MESSAGE, clientfd, buf);
		
		list<int>::iterator it;
		for(it = clients_list.begin(); it != clients_list.end(); it++)
		{
			if(*it != clientfd)
			{
				if(send(*it, message, BUF_SIZE, 0) < 0)
				{
					perror("error");
					exit(-1);
				}
			}
		}
		return len;
	}
}
#endif 
