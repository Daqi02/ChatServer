#include "offlinemessagemodel.h"
#include "mysqlconnectionpool.h"

bool OfflineMessageModel::insert(OfflineMessage &offmsg)
{
    char sql[1024];
    bzero(sql, 1024);
    sprintf(sql, "INSERT INTO OfflineMessage(userid, message) values(%d, '%s')", offmsg.getId(), offmsg.getMessage().c_str());

    /*MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }*/

    shared_ptr<MySQL> mysql = MysqlConnectionPool::instance()->getMySQL();
    if (mysql->update(sql))
    {
        return true;
    }

    return false;
}

vector<string> OfflineMessageModel::query(int id)
{
    char sql[1024];
    bzero(sql, 1024);
    sprintf(sql, "SELECT message FROM OfflineMessage WHERE userid=%d", id);

    vector<string> msgs;

    /*MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            // 获取全部离线消息
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                msgs.push_back(row[0]);
            }
        }
        mysql_free_result(res);
    }*/

    shared_ptr<MySQL> mysql = MysqlConnectionPool::instance()->getMySQL();
    MYSQL_RES *res = mysql->query(sql);
    if (res != nullptr)
    {
        MYSQL_ROW row;
        // 获取全部离线消息
        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            msgs.push_back(row[0]);
        }
    }
    mysql_free_result(res);

    return msgs;
}

void OfflineMessageModel::remove(int id)
{
    char sql[1024];
    bzero(sql, 1024);
    sprintf(sql, "DELETE FROM OfflineMessage WHERE userid=%d", id);

    /*MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }*/

    shared_ptr<MySQL> mysql = MysqlConnectionPool::instance()->getMySQL();
    mysql->update(sql);
}