//
// Created by Sixzeroo on 2018/6/7.
//

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "client.h"
#include "log.h"
#include "config.h"

ChatRoomClient::ChatRoomClient()
{
    _server_ip = "";
    _server_port = -1;
    _client_fd = -1;
    _epollfd = -1;
}

void ChatRoomClient::set_server_ip(const std::string &_server_ip) {
    LOG(DEBUG)<<"Set sever ip: "<<_server_ip<<std::endl;
    ChatRoomClient::_server_ip = _server_ip;
}

void ChatRoomClient::set_server_port(int _server_port) {
    LOG(DEBUG)<<"Set sever port: "<<_server_port<<std::endl;
    ChatRoomClient::_server_port = _server_port;
}

int ChatRoomClient::connect_to_server(std::string server_ip, int port) {
    if(_client_fd != -1)
    {
        LOG(ERROR)<<"Connect to server: epoll had created"<<std::endl;
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
        LOG(ERROR)<<"Connect to server: create socket error"<<std::endl;
        return -1;
    }

    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        LOG(ERROR)<<"Connect to server: connnect to server error"<<std::endl;
        return -1;
    }

    _client_fd = sockfd;
    return 0;
}

int ChatRoomClient::set_noblocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag < 0)
        flag = 0;
    int ret = fcntl(fd, F_SETFL, flag | O_NONBLOCK);
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

std::string ChatRoomClient::get_time_str() {
    time_t tm;
    time(&tm);
    char time_string[128];
    ctime_r(&tm, time_string);
    std::string s_time_string = time_string;
    s_time_string.pop_back();
    s_time_string = "[" + s_time_string + "]";

    return s_time_string;
}

int ChatRoomClient::work_loop() {
    if(_epollfd != -1)
    {
        LOG(ERROR)<<"Start client error: epoll created"<<std::endl;
        return -1;
    }

    int pipefd[2];
    if(pipe(pipefd) < 0)
    {
        LOG(ERROR)<<"Start client error: create pipe error"<<std::endl;
        return -1;
    }

    _epollfd = epoll_create(1024);
    LOG(DEBUG)<<"Start client: create epoll : "<<_epollfd<<std::endl;
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
        LOG(ERROR)<<"Start client: fork error"<<std::endl;
        return -1;
    }
    else if(pid == 0)
    {
        LOG(DEBUG)<<"Client: create child successful"<<std::endl;
        close(pipefd[0]);
        LOG(DEBUG)<<"Client: child process close pipefd[0] successful"<<std::endl;
        char message[BUFF_SIZE];

        printf("Please input '%s' to exit chat room\n", EXIT_MSG);

        while (isworking)
        {
            bzero(message, BUFF_SIZE);
            fgets(message, BUFF_SIZE, stdin);

            if(strncasecmp(message, EXIT_MSG, strlen(EXIT_MSG)) == 0)
            {
                LOG(INFO)<<"Client exit"<<std::endl;
                isworking = false;
            }
            if(write(pipefd[1], message, strlen(message)) < 0)
            {
                LOG(ERROR)<<"Client child process: write error"<<std::endl;
                return -1;
            }

        }
    }
    else
    {
        close(pipefd[1]);

        LOG(DEBUG)<<"Client: parent process close pipefd[1] successful"<<std::endl;

        bool chang_name_flag = false;

        while (isworking)
        {
            int epoll_event_count = epoll_wait(_epollfd, events, 2, -1);

            for(int i = 0; i < epoll_event_count; i++)
            {

                if(events[i].data.fd == _client_fd)
                {
                    LOG(DEBUG)<<"Client epoll: get events from sockfd"<<std::endl;
                    Msg recv_m;
                    recv_m.recv_diy(_client_fd);
                    // 处理关闭连接情况
                    if(recv_m.code != M_NORMAL)
                    {
                        LOG(INFO)<<"Client epoll: close connetct"<<std::endl;
                        isworking = false;
                    }
                    std::string time_str = get_time_str();
                    std::cout<<time_str<<recv_m.context<<std::endl<<std::flush;
                }
                else
                {
                    LOG(DEBUG)<<"Client epoll: get events from terminal"<<std::endl;

                    char message[BUFF_SIZE];
                    bzero(message, sizeof(message));
                    ssize_t ret = read(events[i].data.fd, message, BUFF_SIZE);

                    // 改名的第二阶段
                    if(chang_name_flag)
                    {
                        Msg send_m(M_CNAME, message);
                        send_m.send_diy(_client_fd);

                        LOG(DEBUG)<<"Client epoll: send change name msg to server"<<std::endl;
                        chang_name_flag = false;
                        continue;
                    }

                    // 处理客户端退出聊天室的情况
                    if(strncasecmp(message, EXIT_MSG, strlen(EXIT_MSG)) == 0)
                    {
                        isworking = false;
                        Msg send_m(M_EXIT, "exit");
                        send_m.send_diy(_client_fd);
                        LOG(DEBUG)<<"Client epoll: send exit msg to server"<<std::endl;
                        continue;
                    }
                    // 处理改名的情况，设置改名标志
                    else if(strncasecmp(message, CHANGE_NAME_MSG, strlen(CHANGE_NAME_MSG)) == 0)
                    {
                        chang_name_flag = true;
                        continue;
                    }

                    Msg send_m(M_NORMAL, message);
                    send_m.send_diy(_client_fd);

                    LOG(DEBUG)<<"Client epoll: send msg to server"<<std::endl;
                }
            }
        }
    }

    if(pid)
    {
        close(pipefd[0]);
        close(_epollfd);
    }
    else
    {
        close(pipefd[1]);
    }

    return 0;
}

int ChatRoomClient::start_client(std::string ip, int port) {
    set_server_port(port);

    set_server_ip(ip);

    if(connect_to_server(_server_ip, _server_port) < 0)
    {
        LOG(ERROR)<<"Client start: connect to server error"<<std::endl;
        return -1;
    }

    return work_loop();
}

int main()
{
    std::map<std::string, std::string> config;
    get_config_map("client.config", config);
    init_logger("client_log", "debug.log", "info.log", "warn.log", "error.log", "all.log");
    set_logger_mode(1);
    ChatRoomClient client;
    client.start_client(config["ip"], std::stoi(config["port"]));
    return 0;
}