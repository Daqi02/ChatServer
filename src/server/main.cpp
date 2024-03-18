
#include "chatserver.h"
#include "chatservice.h"
#include <signal.h>

void quithandler(int signo)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        exit(-1);
    }

    string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, quithandler);

    EventLoop loop;                                 //epoll事件循环
    InetAddress addr(ip, port);            //绑定地址和端口号
    ChatServer server(&loop, addr, "ChatServer");   //初始化服务器
    server.start();                                 //监听
    loop.loop();                                    //进入事件循环

    return 0;
}