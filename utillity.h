//
// Created by Sixzeroo on 2018/4/7.
//

#ifndef CHARROOM_UTILLITY_H
#define CHARROOM_UTILLITY_H

#include <iostream>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

using namespace std;

list<int> clients_list;


#define SERVER_IP "127.0.0.1"

#define SERVER_PORT 18888

#define EPOLL_SIZE 20

#define BUFF_SIZE 0xFFFF

#define WELCOM_MES "Welcome to Chatroom! Your clientid is %d"

#define CAUTION "There is only one client in this char room"

#define SERVER_MES "Client %d say >> %s"

#define EXIT "exit"

// 设置文件标志为非阻塞
int setnoblocking(int sockfd)
{
    if(fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK))
    {
        perror("fcntl");
        exit(-1);
    }
    return 0;
}

// 在epoll中注册目标文件描述符
void addfd(int epollfd, int fd, bool enable_et)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    // 允许读
    ev.events = EPOLLIN;
    // 设置为边缘出发模式
    if(enable_et)
    {
        ev.events = EPOLLIN | EPOLLET;
    }
    // 添加进epoll中
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    setnoblocking(fd);
    printf("fd %d add to epoll!\n", fd);
}

int sendBroadcaseMess(int clientfd)
{
    // buf 保存发送来的信息，message是发送给客户端的信息
    char message[BUFF_SIZE], buf[BUFF_SIZE];
    bzero(message, BUFF_SIZE);
    bzero(buf, BUFF_SIZE);

    printf("recv from client (clientID = %d)\n", clientfd);
    int len = recv(clientfd, buf, BUFF_SIZE, 0);

    // 表示关闭client
    if(len == 0)
    {
        // 关闭连接，移除客户端
        close(clientfd);
        clients_list.remove(clientfd);
        printf("ClientID = %d closed.\n Now there are %d clients in char room \n", clientfd, (int)clients_list.size());
    }
    else // 广播通知
    {
        // 只有一个客户端的时候发送警告信息
        if(clients_list.size() == 1)
            send(clientfd, CAUTION, strlen(CAUTION), 0);
        else
        {
            // 为每一个客户端发送信息
            for(auto it:clients_list)
            {
                if(it == clientfd) continue;

                sprintf(message, SERVER_MES, clientfd, buf);
                if(send(it, message, BUFF_SIZE, 0) < 0)
                {
                    perror("send mes");
                    exit(-1);
                }
            }
        }
    }
    return len;
}

#endif //CHARROOM_UTILLITY_H
