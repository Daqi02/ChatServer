#pragma once

#include <muduo/net/TcpConnection.h>
#include <muduo/base/Logging.h>
#include <unordered_map>
#include <functional>
#include <json.hpp>
#include <string>
#include <mutex>
#include <vector>

#include "usermodel.h"
#include "offlinemessagemodel.h"
#include "friendmodel.h"
#include "groupmodel.h"
#include "redis.h"

using muduo::net::TcpConnectionPtr;
using muduo::Timestamp;
using json = nlohmann::json;
using std::unordered_map;
using std::string;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::mutex;
using std::lock_guard;
using std::vector;

using MsgHandler = std::function<void(const TcpConnectionPtr& tcpconn, json& js, Timestamp time)>;

/**
 * 业务类 单例模式
 **/
class ChatService
{
public:
    static ChatService* instance();
    MsgHandler getHandler(int msgID);
    void login(const TcpConnectionPtr& tcpconn, json& js, Timestamp time);      //登录业务
    void regist(const TcpConnectionPtr& tcpconn, json& js, Timestamp time);     //注册业务
    void logoutException(const TcpConnectionPtr& tcpconn);                      //异常退出
    void chat(const TcpConnectionPtr& tcpconn, json& js, Timestamp time);       //聊天业务
    void addfriend(const TcpConnectionPtr& tcpconn, json& js, Timestamp time);  //添加好友
    void createGroup(const TcpConnectionPtr& tcpconn, json& js, Timestamp time);//创建群聊
    void addGroup(const TcpConnectionPtr& tcpconn, json& js, Timestamp time);   //加入群聊
    void chatGroup(const TcpConnectionPtr& tcpconn, json& js, Timestamp time);  //群聊聊天
    void logout(const TcpConnectionPtr& tcpconn, json& js, Timestamp time);     //注销业务
    void reset();       //服务器异常退出重置用户状态信息
    void handelRedisSubscribeMessage(int id, string msg);

private:
    ChatService();

private:
    unordered_map<int, MsgHandler> MsgHandlerMap;   //业务id到业务处理方法的映射
    unordered_map<int, TcpConnectionPtr> ConnMap;   //用户id到连接的映射->长连接
    
    UserModel usermodel;                //数据模型->User用户
    OfflineMessageModel offmsgmodel;    //数据模型->OfflineMessage离线消息
    FriendModel friendmodel;            //数据模型->Friend好友
    GroupModel groupmodel;              //数据模型->Group群聊

    mutex connMapMutex;                 //保证ConnMap互斥访问

    Redis redis;     
};