//
// Created by Sixzeroo on 2018/4/7.
//

#include "utillity.h"

int main(int argc, char *argv[])
{
    // 网络地址结构
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    // 本机字节序转换为网络字节序（大端模式）
    server_addr.sin_port = htons(SERVER_PORT);
    // 处理服务器IP，将字符串格式的IP转换为网络字节序的二进制地址
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);

    // 创建监听套接字
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        perror("listener");
        exit(-1);
    }
    printf("listener listend\n\n");

    // 套接字地址绑定，地址格式强制转化为通用地址格式sockaddr
    if(bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        perror("bind");
        exit(-1);
    }

    // 设定监听，20表示希望入队的未完成请求的数量
    if(listen(listener, 20) < 0)
    {
        perror("listen");
        exit(-1);
    }
    printf("Starting to listen: %s\n\n", SERVER_IP);

    // 创建epoll实例
    int epollfd = epoll_create(EPOLL_SIZE);
    if(epollfp < 0)
    {
        perror("epoll_create");
        exit(-1);
    }
    printf("epoll created, epollfd=%d\n\n", epollfd);
    static struct epoll_event events[EPOLL_SIZE];

    // 在epoll中注册指定的文件描述符
    addfd(epollfd, listener, true);

    // 关闭标记
    bool close_flag = false;

    while(1)
    {
        // 监控发生的事件，可能会永久阻塞
        int epoll_events_count = epoll_wait(epollfd, events, EPOLL_SIZE, -1);

        if(epoll_events_count < 0)
        {
            perror("epoll");
            exit(-1);
        }

        printf("epoll events num is %d", epoll_events_count);

        for(int i=0; i < epoll_events_count; i++)
        {
            int sockfd = events[i].data.fd;

            // 服务器处理逻辑
            if(sockfd == listener)
            {
                // 接受客户端发来的请求
                struct sockaddr_in client_add;
                socklen_t client_add_len = sizeof(struct sockaddr_in);
                int clientfd = accept(listener, (struct sockaddr *)&client_add, &client_add_len);
                // 打印信息，将网络字节序二进制地址转化为字符串格式
                printf("Client connection from %s:%d (IP:Port), clientfd = %d\n",
                    inet_ntoa(client_add.sin_addr),
                    ntohs(client_add.sin_port),
                    clientfd);

                addfd(epollfd, clientfd, true);

                // 加入队列中
                clients_list.push_back(clientfd);
                printf("Add new Client (clientfd = %d) to epoll\n", clientfd);
                printf("Now there are %d clients in ChatRoom \n", (int)clients_list.size());

                printf("Welcome message\n");
                // 写入欢迎信息
                char message[BUFF_SIZE];
                bzero(message, BUFF_SIZE);
                sprintf(message, WELCOM_MES, clientfd);
                if(send(clientfd, message, BUFF_SIZE, 0) < 0)
                {
                    perror("send");
                    exit(-1);
                }
            }
            else
            {
                int ret = sendBroadcaseMess(sockfd);
                if(ret < 0)
                {
                    perror("send mes");
                    exit(-1);
                }
                else if (ret == 0)
                {
                    close_flag = true;
                    break;
                }
            }
        }
        if(close_flag)
            break;
    }

    close(listener);
    close(epollfd);

    return 0;
}