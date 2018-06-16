//
// Created by Sixzeroo on 2018/6/6.
//

#ifndef CHARROOM_PARSE_H
#define CHARROOM_PARSE_H

#include <string>

#define MSG_CHAR_SIZE 4000
#define BUFF_SIZE 4096

enum MsgType {
    M_NORMAL = 1,
    M_EXIT = 2,
    M_CNAME = 3
};

// 消息格式
struct MsgPacket {
    // 消息类型
    int code;
    char context[MSG_CHAR_SIZE];
};

class Msg {
public:
    Msg() = default;

    Msg(int code, std::string context);

    int code;
    std::string context;

    int send_diy(int fd);

    int recv_diy(int fd);
};

#endif //CHARROOM_PARSE_H
