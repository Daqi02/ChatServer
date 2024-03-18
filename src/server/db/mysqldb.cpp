#include "mysqldb.h"

static string server = "127.0.0.1";
static string user = "root";
static string password = "root";
static string dbname = "Chat";

MySQL::MySQL()
{
    conn = mysql_init(nullptr);
}

MySQL::~MySQL()
{
    if(conn != nullptr)
    {
        mysql_close(conn);
    }
}

bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(conn, server.c_str(), user.c_str(), 
        password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if(p != nullptr)
    {
        mysql_query(conn, "set names gbk");
    }
    else
    {
        LOG_INFO << "Connect mysql fail";
    }
    return p;
}

bool MySQL::update(const string& sql)
{
    if(mysql_query(conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败";
        return false;
    }
    return true;
}

MYSQL_RES* MySQL::query(const string& sql)
{
    if(mysql_query(conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败";
        return nullptr;
    }
    return mysql_use_result(conn);
}

MYSQL *MySQL::getConnection()
{
    return conn;
}