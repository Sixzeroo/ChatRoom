//
// Created by Sixzeroo on 2018/6/4.
//

#ifndef CHARROOM_SOCKET_EPOLL_H
#define CHARROOM_SOCKET_EPOLL_H

#include <string>
#include <vector>
#include <sys/epoll.h>

enum SocketEpollStatus {
    S_RUN = 0,
    S_REJECT_CONN = 1,
    S_STOP = 2
};

class SocketEpollWatcher {
public:

    virtual int on_accept() = 0;

    virtual int on_readable() = 0;
};

class SocketEpoll {
private:
    int bind_on(unsigned int ip, int port);

    int listen_on();

    int create_epoll();

    int handle_event(epoll_event &e);

    int handle_accept_event(const int &epollfd, epoll_event &event, SocketEpollWatcher &socket_watcher);

    int _epollfd;
    int _max_events;
    std::string _bind_ip;
    int _backlog;
    int _port;
    int _listen_socket;
    int _status;
    std::vector<int> _client_list;
    int _clients;

public:
    SocketEpoll();

    ~SocketEpoll();

    void set_bind_ip(const std::string &_bind_ip);

    void set_backlog(int _backlog);

    void set_port(int _port);

    void set_max_events(int _max_events);

    int set_nonblocking(int fd);

    int add_listen_sock_to_epoll();

    int start_epoll();

    int start_epoll_loop();

    int stop_epoll();


};

#endif //CHARROOM_SOCKET_EPOLL_H
