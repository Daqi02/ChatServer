#pragma once

#include <string.h>
#include "group.h"
#include "mysqldb.h"

class GroupModel
{
public:
    bool createGroup(Group &group);                         //创建群聊
    bool addGroup(int userid, int groupid, string role);   //加入群聊
    vector<Group> queryGroups(int userid);                  //获取用户的群聊
    vector<int> queryGroupUsers(int userid, int groupid);   //获取群聊用户   

private:
};