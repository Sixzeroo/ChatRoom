//
// Created by Sixzeroo on 2018/4/7.
//

#include "utillity.h"

int main(int argc, char* argv[])
{
    // 服务端网络地址结构
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    // 本机字节序转换为网络字节序（大端模式）
    server_addr.sin_port = htons(SERVER_PORT);
    // 处理服务器IP，将字符串格式的IP转换为网络字节序的二进制地址
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);

    // 创建客户端套接字描述符
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("socket create");
        exit(-1);
    }

    // 连接服务端
    if(connect(sockfd, (struct sockaddr *)server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(-1);
    }

    // 创建管道
    int pipefd[2];
    if(pipe(pipefd) < 0)
    {
        perror("pipe create");
        exit(-1);
    }

    // 创建epoll
    int epollfd = epoll_create(EPOLL_SIZE);
    if(epollfd < 0)
    {
        perror("epoll create");
        exit(-1);
    }
    static struct epoll_event events[2];

    addfd(epollfd, sockfd, true);
    // 通过管道进行通讯，读管道
    addfd(epollfd, pipefd[0], true);

    bool isclientwork = true;

    char message[BUFF_SIZE];

    // 启动两个进程，一个进程用来等待用户输入，另一个进程发送
    int pid = fork();
    if(pid < 0)
    {
        perror("fork");
        exit(-1);
    }
    else if(pid == 0)
    {
        // 输入进程，通过管道将信息传给发送进程
        close(pipefd[0]);
        printf("Please input '%s' to exit chat room\n", EXIT);

        while (isclientwork)
        {
            bzero(message, BUFF_SIZE);
            fgets(message, BUFF_SIZE, stdin);

            if(strncasecmp(message, EXIT, strlen(EXIT)) == 0)
            {
                isclientwork = false;
            }
            else
            {
                if(write(pipefd[1], message, strlen(message)-1) < 0)
                {
                    perror("fork error");
                    exit(-1);
                }
            }
        }
    }
    else
    {
        close(pipefd[1]);

        while (isclientwork)
        {
            int epoll_event_count = epoll_wait(epollfd, events, 2, -1);

            for(int i=0; i<epoll_event_count; i++)
            {
                bzero(&message, BUFF_SIZE);

                // 接收服务器发来的信息
                if(events[i].data.fd == sockfd)
                {
                    int ret = recv(sockfd, message, BUFF_SIZE, 0);

                    // 服务器关闭连接发来的信息
                    if(ret == 0)
                    {
                        printf("Server close connection:%d\n", sockfd);
                        close(sockfd);
                        isclientwork = false;
                    }
                    else
                        printf("%s\n", message);
                }
                // 输入进程发来的信息
                else
                {
                    int ret = read(events[i].data.fd, message, BUFF_SIZE);

                    if(ret == 0)
                        isclientwork = false;
                    else
                        send(sockfd, message, BUFF_SIZE, 0);
                }
            }
        }
    }

    // 父进程关闭读管道、和套接字描述符
    if(pid)
    {
        close(pipefd[0]);
        close(sockfd);
    }
    // 子进程关闭写管道
    else
    {
        close([pipefd[1]]);
    }

    return 0;
}