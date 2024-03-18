#pragma once

/**
 * 消息类 CS共享
 **/
enum msg_type
{
    LOGIN_MSG = 1,          //登录消息
    REGIST_MSG = 2,         //注册消息
    REGIST_ACK = 3,         //注册应答
    LOGIN_ACK = 4,          //登录应答
    CHAT_MSG = 5,           //聊天消息
    ADD_FRIEND_MSG = 6,     //添加好友消息
    CREATE_GROUP_MSG = 7,   //创建群聊
    ADD_GROUP_MSG = 8,      //加入群聊
    GROUP_CHAT_MSG = 9,     //群组聊天
    LOGOUT_MSG = 10
};