#include "mysqlconnectionpool.h"

MysqlConnectionPool::MysqlConnectionPool(int _maxconn, int _timeout)
    : maxConn(_maxconn), timeout(_timeout)
{
    for (int i = 0; i < maxConn; ++i)
    {
        MySQL *conn = new MySQL;
        if(conn->connect())
        {
            connectionqueue.emplace(conn);
        }
    }
}

MysqlConnectionPool::~MysqlConnectionPool()
{
    while (!connectionqueue.empty())
    {
        MySQL *conn = connectionqueue.front();
        connectionqueue.pop();
        delete conn;
    }
}

MysqlConnectionPool *MysqlConnectionPool::instance()
{
    static MysqlConnectionPool connpool(100, 1000);
    return &connpool;
}

std::shared_ptr<MySQL> MysqlConnectionPool::getMySQL()
{
    std::unique_lock<std::mutex> locker(mymutex);
    while (connectionqueue.empty())
    {
        // 如果为空，需要阻塞一段时间，等待新的可用连接
        if (std::cv_status::timeout == mycond.wait_for(locker, std::chrono::milliseconds(timeout)))
        {
            if (connectionqueue.empty())
            {
                continue;
            }
        }
    }

    // 指定删除器
    // 归还连接
    std::shared_ptr<MySQL> connptr(connectionqueue.front(),
                                   [this](MySQL *conn)
                                   {
                                       lock_guard<std::mutex> locker(mymutex); // 互斥访问
                                       connectionqueue.push(conn);             // 归还连接
                                   });
    connectionqueue.pop();
    mycond.notify_all();
    return connptr;
}

int MysqlConnectionPool::getFreeConnNum()
{
    return connectionqueue.size();
}

int MysqlConnectionPool::getMaxConnNum()
{
    return maxConn;
}

int MysqlConnectionPool::getBusyConnNum()
{
    return maxConn - connectionqueue.size();
}