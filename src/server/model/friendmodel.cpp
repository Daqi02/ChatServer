#include "friendmodel.h"

bool FriendModel::insert(Friend user)
{
    char sql[1024];
    bzero(sql, 1024);
    sprintf(sql, "INSERT INTO Friend(userid, friendid) values(%d, %d)",
        user.getUserId(), user.getFriendId());

    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

bool FriendModel::remove(Friend user)
{
    char sql[1024];
    bzero(sql, 1024);
    sprintf(sql, "DELETE FROM OfflineMessage WHERE userid=%d and friendid=%d", 
        user.getUserId(), user.getFriendId());

    MySQL mysql;
    if (mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

vector<User> FriendModel::query(int uid)
{
    vector<User> friends;
    char sql[1024];
    bzero(sql, 1024);
    sprintf(sql, "SELECT id, name, state FROM Friend JOIN User on friendid=id  WHERE userid=%d", uid);

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            //获取全部离线消息
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                friends.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return friends;
}