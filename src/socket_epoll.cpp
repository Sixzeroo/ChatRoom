//
// Created by Sixzeroo on 2018/6/4.
//

#include <cstdlib>
#include <cstdio>
#include <string>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "socket_epoll.h"

SocketEpoll::SocketEpoll(){
    _epollfd = -1;
    _bind_ip = "";
    _listen_socket = -1;
    _port = -1;
    _clients = 0;
    _status = 0;  // 构造时的状态为停止状态
    _backlog = 20;
    _max_events = 100;
}

SocketEpoll::~SocketEpoll(){
    stop_epoll();
}

void SocketEpoll::set_port(int _port) {
    SocketEpoll::_port = _port;
}

void SocketEpoll::set_max_events(int _max_events) {
    SocketEpoll::_max_events = _max_events;
}

void SocketEpoll::set_bind_ip(const std::string &_bind_ip) {
    SocketEpoll::_bind_ip = _bind_ip;
}

void SocketEpoll::set_backlog(int _backlog) {
    SocketEpoll::_backlog = _backlog;
}

/*
 * 绑定到指定ip port
 * ip是二进制形式地址
 * 正确是返回0，出错时返回-1
 */
int SocketEpoll::bind_on(unsigned int ip, int port) {
    // 已经存在
    if(_listen_socket != -1)
    {
        // LOG ERROR
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        // LOG ERROR
        return -1;
    }

    // https://blog.csdn.net/adrianfeng/article/details/6010686
    // 设置重用端口
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 网络地址结构
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    // 本机字节序转换为网络字节序（大端模式）
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = ip;

    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        // LOG ERROR
        return -1;
    }

    if(listen(sockfd, _backlog) == -1)
    {
        // LOG ERROR
        return -1;
    }

    _listen_socket = sockfd;
    return 0;
}

int SocketEpoll::listen_on() {
    int port = 8888;
    if(_port != -1)
        port = _port;
    if(_bind_ip == "")
    {
        int ret = bind_on(INADDR_ANY, port);
        if(ret != 0)
            return ret;
    }
    else
    {
        unsigned int ip = inet_addr(_bind_ip.c_str());
        int ret = bind_on(ip, port);
        if(ret != 0)
            return ret;
    }
    return 0;
}

int SocketEpoll::create_epoll() {
    if(_epollfd != -1)
    {
        // LOG ERROR
        return -1;
    }

    int epollfd = epoll_create(1024);
    if(epollfd == -1)
    {
        // LOG ERROR
        return -1;
    }
    _epollfd = epollfd;
    return 0;
}

int SocketEpoll::handle_event(epoll_event &e) {
    if(e.data.fd == _listen_socket)
    {
        // 处理接受请求
        handle_accept_event(_epollfd, e, _watcher);
    }
    else if(e.events & EPOLLIN)
    {
        // 处理来信请求
        handle_readable_event(e, _watcher);
    }
    else
    {
        // LOG INFO
    }
    return 0;
}

int SocketEpoll::handle_accept_event(const int &epollfd, epoll_event &event, SocketEpollWatcher *socket_watcher) {
    int sockfd = event.data.fd;

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);
    int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);

    std::string client_ip = inet_ntoa(client_addr.sin_addr);
    int client_port = ntohs(client_addr.sin_port);

    // epoll_event 中自带内容
    EpollContext *epoll_context = new EpollContext();
    epoll_context->fd = client_fd;
    epoll_context->client_ip = client_ip;
    epoll_context->client_port = client_port;

    // TODO: 处理accept的额外事件
    socket_watcher->on_accept(*epoll_context);

    // TODO: 处理epoll event中所包含的内容
    struct epoll_event ev;
    ev.data.fd = client_fd;
    ev.data.ptr = epoll_context;
    ev.events = EPOLLIN | EPOLLET;  // 边沿出发模式
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
    {
        // LOG ERROR
        return -1;
    }
    set_nonblocking(client_fd);
    // 加入客户端队列中
    _client_list.push_back(client_fd);
    return 0;
}

int SocketEpoll::handle_readable_event(epoll_event &event, SocketEpollWatcher *socket_watcher) {
    EpollContext *epoll_context = (EpollContext *)event.data.ptr;
    int fd = event.data.fd;

    int ret = socket_watcher->on_readable(*epoll_context, _client_list);

    if(ret == -1)
    {
        // LOG ERROR
        return  -1;
    }
    return 0;
}

// 设置非阻塞
int SocketEpoll::set_nonblocking(int fd) {
    int ret = fcntl(fd, F_GETFL, 0);
    if(ret == -1) ret = 0;
    return fcntl(fd, F_SETFL, ret | O_NONBLOCK);
}

// 将监听端口的Socket套接字加入epoll
int SocketEpoll::add_listen_sock_to_epoll() {
    if(_epollfd == -1 || _listen_socket == -1)
    {
        // LOG ERROR
        return -1;
    }

    struct epoll_event ev;
    ev.data.fd = _listen_socket;
    ev.events = EPOLLIN;  // 边沿出发模式
    if(epoll_ctl(_epollfd, EPOLL_CTL_ADD, _listen_socket, &ev) == -1)
    {
        // LOG ERROR
        return -1;
    }
    return 0;
}

int SocketEpoll::start_epoll() {
    if(listen_on() == -1)
    {
        // LOG ERROR
        return -1;
    }

    if(create_epoll() == -1)
    {
        // LOG ERROR
        return -1;
    }

    if(add_listen_sock_to_epoll() == -1)
    {
        // LOG ERROR
        return -1;
    }

    return start_epoll_loop();
}

int SocketEpoll::start_epoll_loop() {
    epoll_event *events = new epoll_event[_max_events];
    while (_status != S_STOP)
    {
        int epoll_events_count = epoll_wait(_epollfd, events, _max_events, -1);
        if(epoll_events_count == -1)
        {
            if(errno == EINTR)
            {
                // LOG
                continue;
            }
            // LOG ERROR
            break;
        }

        for(int i = 0; i < epoll_events_count; i++)
            handle_event(events[i]);
    }

    // LOG
    if(events != NULL)
    {
        delete[] events;
        events = NULL;
    }
    return 0;
}

// 设置状态为拒绝连接状态
int SocketEpoll::stop_epoll() {
    _status = S_REJECT_CONN;
}
