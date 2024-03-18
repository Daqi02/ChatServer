#include "redis.h"

Redis::Redis()
    : publish_context(nullptr), subscribe_context(nullptr)
{
}

Redis::~Redis()
{
    if(publish_context != nullptr)
    {
        redisFree(publish_context);
    }
    if(subscribe_context != nullptr)
    {
        redisFree(subscribe_context);
    }
}

bool Redis::connect()
{
    publish_context = redisConnect("127.0.0.1", 6379);
    if(publish_context == nullptr || publish_context->err)
    {
        cerr << "redisConnct() failed : " << publish_context->errstr << endl;
        return false;
    }

    subscribe_context = redisConnect("127.0.0.1", 6379);
    if(subscribe_context == nullptr || subscribe_context->err)
    {
        cerr << "redisConnct() failed : " << publish_context->errstr << endl;
        return false;
    }
    
    //单独的线程监听通道消息 上报消息给业务层
    thread t([&](){
        observer_channel_message();
    });
    t.detach();

    cout << "redisConnect success" << endl;
    return true;
}

bool Redis::publish(int channel, string message)
{
    redisReply *reply = (redisReply *)redisCommand(publish_context, "PUBLISH %d %s", channel, message.c_str());
    if(reply == nullptr)
    {
        cerr << "redisCommad() error : publish_context" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel)
{
    // SUBSCRIBE 命令会造成线程阻塞等待通道消息
    // 因此需要只订阅而不接收消息
    // 接收消息会在专门的线程函数中进行

    char cmd[64];
    sprintf(cmd, "SUBSCRIBE %d", channel);
    
    
    //if(redisAppendCommand(subscribe_context, cmd) == REDIS_ERR);
    //{
    //    cerr << "redisAppendCommand() error : subscribe command failed : " << cmd << endl;
    //    return false;
    //}
    ///////这里被坑惨了 不需要判断返回值 不然总是return false
    //
    redisAppendCommand(subscribe_context, cmd);

    int done = 0;
    while(!done)
    {
        if(redisBufferWrite(subscribe_context, &done) == REDIS_ERR)
        {
            cerr << "redisBufferWrite() error : subscribe command failed" << endl;
            return false;
        }
    }

    // 实际上redisCommand() 做了 redisAppendCommand() redisBufferWrite() redisGetReply() 的工作
    // redisGetReply() 不需要得到消息 不发生阻塞

    return true;
}

bool Redis::unsubscribe(int channel)
{
    if(redisAppendCommand(this->subscribe_context, "UNSUBSCRIBE %d", channel) == REDIS_ERR);
    {
        cerr << "redisAppendCommand() error : unsubscribe command failed" << endl;
        return false;
    }

    int done = 0;
    while(!done)
    {
        if(redisBufferWrite(this->subscribe_context, &done) == REDIS_ERR)
        {
            cerr << "redisBufferWrite() error : unsubscribe command failed" << endl;
            return false;
        }
    }

    return true;
}

void Redis::observer_channel_message()
{
    redisReply *reply = nullptr;
    while(redisGetReply(this->subscribe_context, (void **)&reply) == REDIS_OK)
    {
        //订阅消息是三元组
        if(reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            //上报业务层消息
            notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }

        freeReplyObject(reply);
    }
}

void Redis::init_notify_handler(function<void(int, string)> fn)
{
    this->notify_message_handler = fn;
}
