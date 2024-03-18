#include "chatservice.h"
#include "message.h"

ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

ChatService::ChatService()
{
    MsgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    MsgHandlerMap.insert({REGIST_MSG, std::bind(&ChatService::regist, this, _1, _2, _3)});
    MsgHandlerMap.insert({CHAT_MSG, std::bind(&ChatService::chat, this, _1, _2, _3)});
    MsgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addfriend, this, _1, _2, _3)});
    MsgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    MsgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)}); 
    MsgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::chatGroup, this, _1, _2, _3)}); 
    MsgHandlerMap.insert({LOGOUT_MSG, std::bind(&ChatService::logout, this, _1, _2, _3)});

    //Redis发布订阅用作MQ
    //连接redis服务器
    if(redis.connect())
    {
        // 设置上报消息回调
        redis.init_notify_handler(std::bind(&ChatService::handelRedisSubscribeMessage, this, _1, _2));
    }
}

MsgHandler ChatService::getHandler(int msgID)
{
    auto handler = MsgHandlerMap.find(msgID);

    // 获取消息ID对应的处理器
    if (handler == MsgHandlerMap.end())
    {
        // 返回一个记录错误日志的默认处理器
        return [=](const TcpConnectionPtr &tcpconn, json &js, Timestamp time)
        {
            LOG_ERROR << "MSGID ERROR: " << msgID << " NOT EXISTS";
        };
    }
    else
    {
        return MsgHandlerMap[msgID];
    }
}

// 处理登录业务: 更新在线状态 -> 查询离线消息
void ChatService::login(const TcpConnectionPtr &tcpconn, json &js, Timestamp time)
{
    LOG_INFO << "LOGIN!!";

    // 反序列化json
    int id = js["id"].get<int>();
    string pwd = js["pwd"];

    User user = usermodel.query(id);
    json response;
    response["msgid"] = LOGIN_ACK;
    if (user.getId() != -1 && user.getPassword() == pwd)
    {
        if (user.getState() == "online")
        {
            // 已经在一台设备登录
            // 填充应答消息
            response["errno"] = 2;
            response["errmsg"] = "this account have been logined in other computor";
        }
        else
        {
            //登录成功
            //保存连接 ---> 这里需要加锁线程间互斥访问
            {
                lock_guard<mutex> lock(connMapMutex);
                ConnMap.insert({user.getId(), tcpconn});
            }

            //成功登录 向redis订阅channel(/id)
            redis.subscribe(id);

            //填充应答消息
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            //更新用户信息
            user.setState("online");
            usermodel.updateState(user);

            //查询离线消息 暂未实现 2024.3.14
            //实现 2024.3.15
            //查询数据库并将所有离线消息放入vector容器中
            vector<string> msgs = offmsgmodel.query(id);
            if(!msgs.empty())
            {
                response["offlinemsg"] = msgs;
                //删除离线消息
                offmsgmodel.remove(id);
            }

            //查询好友信息
            vector<User> friends = friendmodel.query(id);
            vector<string> jss;
            for(const User& f : friends)
            {
                json tmpjs;
                tmpjs["id"] = f.getId();
                tmpjs["name"] = f.getName();
                tmpjs["state"] = f.getState();
                jss.emplace_back(tmpjs.dump());
            }
            response["friends"] = jss;

            //查询群组信息
            vector<Group> groups = groupmodel.queryGroups(id);
            if(!groups.empty())
            {
                vector<string> groupV;
                for(Group &group : groups)
                {
                    json groupjs;
                    groupjs["id"] = group.getId();
                    groupjs["groupname"] = group.getName();
                    groupjs["groupdesc"] = group.getDesc();
                    vector<string> groupusers;
                    for(GroupUser &groupuser : group.getUsers())
                    {
                        json gujs;
                        gujs["id"] = groupuser.getId();
                        gujs["name"] = groupuser.getName();
                        gujs["state"] = groupuser.getState();
                        gujs["role"] = groupuser.getRole();
                        groupusers.push_back(gujs.dump());
                    }
                    groupjs["users"] = groupusers;
                    groupV.push_back(groupjs.dump());
                }
                response["groups"] = groupV;
            }
            
            //发送应答消息
            tcpconn->send(response.dump());
        }
    }
    else
    {
        // 登陆失败
        // 填充应答消息
        response["errno"] = 1;
        response["errmsg"] = "userid or password is not correct";
        // 发送应答消息
        tcpconn->send(response.dump());
    }
}

// 处理账户注册业务: 写入数据库 -> 返回id给用户
void ChatService::regist(const TcpConnectionPtr &tcpconn, json &js, Timestamp time)
{
    LOG_INFO << "REGIST!!";

    // 反序列化json
    string name = js["name"];
    string pwd = js["pwd"];

    User user;
    user.setName(name);
    user.setPassword(pwd);
    bool res = usermodel.insert(user);

    json response;
    response["msgid"] = REGIST_ACK;
    if (res)
    {
        // 注册成功
        response["id"] = user.getId();
        response["errno"] = 0;
    }
    else
    {
        // 注册失败
        response["errno"] = 1;
    }
    tcpconn->send(response.dump());
}

// 处理用户异常退出: Map删除连接 -> 更新离线状态
void ChatService::logoutException(const TcpConnectionPtr &tcpconn)
{
    User user;

    {
        lock_guard<mutex> lock(connMapMutex);

        // 此处可做优化 -> 再创建一个Map用于保存conn->id的键值对
        // 以空间换时间 -> 不需要再遍历 -> 直接erase(conn) -> 加快查找
        // 后续再做优化 --- 2024.3.14
        for (auto it = ConnMap.begin(); it != ConnMap.end(); ++it)
        {
            if (it->second == tcpconn)
            {
                // 删除该连接
                ConnMap.erase(it);
                user.setId(it->first);
                break;
            }
        }
    }

    //异常退出 同样需要取消订阅
    redis.unsubscribe(user.getId());

    // 更新状态为离线
    if (user.getId() != -1)
    {
        user.setState("offline");
        usermodel.updateState(user);
    }
}

// 发送聊天消息业务: 对方在线->转发  对方离线->暂存数据库
void ChatService::chat(const TcpConnectionPtr &tcpconn, json &js, Timestamp time)
{
    // 接收者id
    int to = js["to"].get<int>();

    {
        lock_guard<mutex> lock(connMapMutex);
        auto it = ConnMap.find(to);

        if (it != ConnMap.end())
        {
            // 对方在线 -> 转发 主动推送给对方
            it->second->send(js.dump());
            return;
        }
    }

    // 对方不在线 -> 数据库存储离线消息 暂未实现 2024.3.14
    // 增加了离线消息数据模型类 实现 2024.3.15
    ////////
    // 更新 : 对方可能在其他服务器登录 2024.3.18
    // 查看对方的在线状态
    User user = usermodel.query(to);
    if(user.getState() == "online")
    {
        redis.publish(to, js.dump());
        return;
    }

    // 不在线 存入离线消息
    OfflineMessage offmsg(to, js.dump());
    offmsgmodel.insert(offmsg);
}

// 重置用户登录状态信息
void ChatService::reset()
{
    usermodel.resetState();
}

// 添加好友
void ChatService::addfriend(const TcpConnectionPtr& tcpconn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    //存储好友信息
    friendmodel.insert(Friend(userid, friendid));
}

//创建群组
void ChatService::createGroup(const TcpConnectionPtr& tcpconn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    Group group(-1, name, desc);
    if(groupmodel.createGroup(group))
    {
        groupmodel.addGroup(userid, group.getId(), "creator");
    }
}

//加入群组
void ChatService::addGroup(const TcpConnectionPtr& tcpconn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    groupmodel.addGroup(userid, groupid, "normal");
}

//群组聊天
void ChatService::chatGroup(const TcpConnectionPtr& tcpconn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> userids = groupmodel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(connMapMutex);
    for(int id : userids)
    {
        auto it = ConnMap.find(id);
        if(it != ConnMap.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            //2024.3.18 引入redis作为消息队列
            //查询在其他服务器是否登录
            User user = usermodel.query(id);
            if(user.getState() == "online")
            {
                redis.publish(id, js.dump());
            }
            else
            {
                //不在线 存储离线消息
                OfflineMessage msg(id, js.dump());
                offmsgmodel.insert(msg);
            }
        }
    }
}

//注销
void ChatService::logout(const TcpConnectionPtr& tcpconn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(connMapMutex);
        auto it = ConnMap.find(userid);
        if(it != ConnMap.end())
        {
            ConnMap.erase(it);
        }
    }

    //用户注销下线 需要取消订阅
    redis.unsubscribe(userid);

    //更新用户状态
    User user(userid, "", "", "offline");
    usermodel.updateState(user);
}

void ChatService::handelRedisSubscribeMessage(int id, string msg)
{
    lock_guard<mutex> lock(connMapMutex);
    auto it = ConnMap.find(id);
    if(it != ConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    //存储离线消息
    OfflineMessage offmsg(id, msg);
    offmsgmodel.insert(offmsg);
}
