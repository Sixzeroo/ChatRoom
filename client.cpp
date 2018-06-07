//
// Created by Sixzeroo on 2018/6/7.
//

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "client.h"

ChatRoomClient::ChatRoomClient()
{
    _server_ip = "";
    _server_port = -1;
    _client_fd = -1;
}

void ChatRoomClient::set_server_ip(const std::string &_server_ip) {
    ChatRoomClient::_server_ip = _server_ip;
}

void ChatRoomClient::set_server_port(int _server_port) {
    ChatRoomClient::_server_port = _server_port;
}

int ChatRoomClient::connect_to_server(std::string server_ip, int port) {
    if(_client_fd != -1)
    {
        // LOG ERROR
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    unsigned int ip = inet_addr(server_ip.c_str());
    server_addr.sin_addr.s_addr = ip;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        // LOG ERROR
        return -1;
    }

    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        // LOG ERROR
        return -1;
    }

    _client_fd = sockfd;
    return 0;
}

int ChatRoomClient::set_noblocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag < 0)
        flag = 0;
    int ret = fcntl(fd, F_SETFL, flag | O_NONBLOCK)
    if(ret < 0)
    {
        // LOG ERROR
        return -1;
    }
    return 0;
}

int ChatRoomClient::addfd(int epollfd, int fd, bool enable_et) {
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
    set_noblocking(fd);
    return 0;
}

int ChatRoomClient::work_loop() {
    if(_epollfd != -1)
    {
        // LOG ERROR
        return -1;
    }

    int pipefd[2];
    if(pipe(pipefd) < 0)
    {
        // LOG ERROR
        return -1;
    }

    _epollfd = epoll_create(1024);
    if(_epollfd < 0)
    {
        // LOG ERROR
        return -1;
    }
    static struct epoll_event events[2];

    addfd(_epollfd, _client_fd, true);
    addfd(_epollfd, pipefd[0], true);

    bool isworking = true;

    int pid = fork();
    if(pid < 0)
    {
        // LOG ERROR
        return -1;
    }
    else if(pid == 0)
    {
        close(pipefd[0]);
        char message[BUFF_SIZE];

        bzero(message, BUFF_SIZE);
        fgets(message, BUFF_SIZE, stdin);

        if(strncasecmp(message, EXIT_MSG, strlen(EXIT_MSG)) == 0)
        {
            isworking = false;
        }
        else
        {
            if(write(pipefd[1], message, strlen(message)) < 0)
            {
                perror("fork error");
                exit(-1);
            }
        }
    }
    else
    {
        close(pipefd[1]);

        while (isworking)
        {
            int epoll_event_count = epoll_wait(_epollfd, events, 2, -1);

            for(int i = 0; i < epoll_event_count; i++)
            {
                if(events[i].data.fd = _client_fd)
                {
                    Msg recv_m;
                    recv_m.recv_diy(_client_fd);
                    // 处理关闭连接情况
                    if(recv_m.code != M_NORMAL)
                    {
                        // LOG ERROR
                        return -1;
                    }
                    std::cout<<recv_m.context;
                }
                else
                {
                    char message[BUFF_SIZE];
                    ssize_t ret = read(events[i].data.fd, message, BUFF_SIZE);

                    Msg send_m(M_NORMAL, message);
                    send_m.send_diy(_client_fd);
                }
            }
        }
    }
}

int ChatRoomClient::start_client() {
    // 设置默认端口
    if(_server_port == -1)
        set_server_port(8888);

    // 设置默认IP地址
    if(_server_ip.empty())
        set_server_ip("127.0.0.1");

    return work_loop();
}