#pragma once

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
#include <string>
#include <iostream>

using std::function;
using std::thread;
using std::string;
using std::cerr;
using std::cout;
using std::endl;

class Redis
{
public:
    Redis();
    ~Redis();

    bool connect();                                             //连接redis服务器
    bool publish(int channel, string message);                  //发布消息
    bool subscribe(int channel);                                //订阅消息                                         
    bool unsubscribe(int channel);                              //取消订阅

    void observer_channel_message();                            //在独立线程中接收订阅通道的消息
    void init_notify_handler(function<void(int, string)> fn);    //初始化业务层上报通道消息的回调对象

private:
    redisContext *publish_context;                              //负责发布消息
    redisContext *subscribe_context;                            //负责订阅消息
    function<void(int, string)> notify_message_handler;         //回调操作 收到订阅消息后给service层上报
};