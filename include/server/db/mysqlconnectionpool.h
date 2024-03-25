#pragma once

#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <memory>

#include "mysqldb.h"

using std::condition_variable;
using std::lock_guard;
using std::mutex;
using std::queue;
using std::shared_ptr;
using std::vector;

class MysqlConnectionPool
{
private:
    int maxConn;                    // 最大连接数
    queue<MySQL *> connectionqueue; // 连接池

private:
    MysqlConnectionPool(int _maxconn, int _timeout);
    MysqlConnectionPool(const MysqlConnectionPool &obj) = delete;
    MysqlConnectionPool(const MysqlConnectionPool &&obj) = delete;
    MysqlConnectionPool &operator=(const MysqlConnectionPool &obj) = delete;
    mutex mymutex;
    condition_variable mycond;
    int timeout;

public:
    ~MysqlConnectionPool();
    static MysqlConnectionPool *instance();
    shared_ptr<MySQL> getMySQL(); // 获取一个连接
    int getFreeConnNum();         // 获取空闲连接数
    int getMaxConnNum();          // 获取最大连接数
    int getBusyConnNum();         // 获取忙碌连接数
};