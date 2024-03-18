#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <string>
#include <functional>
#include "json.hpp"
#include "chatservice.h"

using std::cout;
using std::endl;
using std::string;
using std::function;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using muduo::net::TcpServer;
using muduo::net::EventLoop;
using muduo::net::InetAddress;
using muduo::net::TcpConnectionPtr;
using muduo::net::Buffer;
using muduo::Timestamp;
using json = nlohmann::json;

/**
 * 基于muduo库的chat服务器
 **/
class ChatServer
{
public:
    ChatServer(EventLoop* _loop, const InetAddress& laddr, const string& name);

    void start();


private:
    TcpServer server;   //muduo库服务器对象
    EventLoop *loop;    //主线程事件循环

    void handle_conn(const TcpConnectionPtr& tcpconn);
    void handle_msg(const TcpConnectionPtr& tcpconn, Buffer *buf, Timestamp time);
};