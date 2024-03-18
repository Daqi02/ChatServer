#pragma once

#include <mysql/mysql.h>
#include <string>
#include <muduo/base/Logging.h>

using std::string;

/**
 *  封装MySQL数据库 
 **/
class MySQL
{
public:
    MySQL();
    ~MySQL();
    bool connect();
    bool update(const string& sql);
    MYSQL_RES *query(const string& sql);
    MYSQL *getConnection();

private:
    MYSQL *conn;
};