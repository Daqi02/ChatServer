#include "chatserver.h"
#include "mysqlconnectionpool.h"

ChatServer::ChatServer(EventLoop* _loop, const InetAddress& laddr, const string& name)
    :server(_loop, laddr, name), loop(_loop)
{
    //注册用户回调函数-> 连接 断开 消息读写
    server.setConnectionCallback(std::bind(&ChatServer::handle_conn, this, _1));
    server.setMessageCallback(std::bind(&ChatServer::handle_msg, this, _1, _2, _3));

    //设置工作线程数量
    server.setThreadNum(4);
}

void ChatServer::start()
{
    MysqlConnectionPool::instance();
    server.start();
}

void ChatServer::handle_conn(const TcpConnectionPtr& tcpconn)
{
    //客户端断开连接
    if(!tcpconn->connected())
    {
        ChatService::instance()->logoutException(tcpconn);
        tcpconn->shutdown();
    }
}

void ChatServer::handle_msg(const TcpConnectionPtr& tcpconn, Buffer *buf, Timestamp time)
{
    //获取消息并反序列json
    string buff = buf->retrieveAllAsString();
    json js = json::parse(buff);

    //  获取消息ID 
    //  -----> ID寻找对应业务回调函数
    //  -----> 不需要了解内部具体业务
    //  -----> 解耦网络模块和业务模块
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    
    //处理业务
    msgHandler(tcpconn, js, time);
}
