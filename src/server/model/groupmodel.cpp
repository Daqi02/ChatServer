#include "groupmodel.h"

bool GroupModel::createGroup(Group &group)
{
    char sql[1024];
    bzero(sql, 1024);
    sprintf(sql, "INSERT INTO AllGroup(groupname, groupdesc) values('%s', '%s')",
        group.getName().c_str(), group.getDesc().c_str());

    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

bool GroupModel::addGroup(int userid, int groupid, string role)
{
    char sql[1024];
    bzero(sql, 1024);
    sprintf(sql, "INSERT INTO GroupUser(groupid, userid, grouprole) values(%d,%d,'%s')",
        groupid, userid, role.c_str());

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

//查询用户所有群组信息
vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[1024];
    bzero(sql, 1024);
    sprintf(sql, "SELECT g.id, g.groupname, g.groupdesc FROM \
        AllGroup as g join GroupUser as u on g.id = u.groupid \
        WHERE u.userid=%d", userid);

    vector<Group> groups;

    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                groups.emplace_back(atoi(row[0]), row[1], row[2]);
            }
        }
        mysql_free_result(res);
    }

    //获取每个群聊的所有成员
    for(Group &group : groups)
    {
        sprintf(sql, "SELECT User.id, User.name, User.state, GroupUser.grouprole \
                FROM User join GroupUser on User.id=GroupUser.userid \
                WHERE GroupUser.groupid=%d", group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                group.getUsers().emplace_back(atoi(row[0]), string(row[1]), string(row[2]), string(row[3]));
            }
        }  
    }

    return groups;
}

vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024];
    bzero(sql, 1024);
    sprintf(sql, "SELECT userid FROM \
        GroupUser \
        WHERE groupid = %d and userid!=%d", groupid, userid);
    
    vector<int> users;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                users.push_back(atoi(row[0]));
            }
        }
        mysql_free_result(res);
    }
    return users;
}