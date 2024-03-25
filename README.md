# ChatServer
基于muduo库并实现Nginx负载均衡的集群聊天服务器ChatServer

# 技术点
1. 使用cmake构建的基于muduo网络库开发的ChatServer服务器；
2. 通过解耦网络模块和业务模块的代码，使得业务代码编写更加轻松；
3. 配置Nginx实现权重轮询的负载均衡，实现聊天集群功能，提高并发；
4. 使用Redis的发布-订阅功能实现跨服务器的聊天通信；
5. 使用关系型数据库MySQL存储数据；
6. 使用第三方库JSON For Modern C++，通过json实现客户端和服务端的通信协议。

# 编译
cd build  
rm -rf *  
cmake ..  
make

也可使用build.sh一键编译构建  
./build.sh

# 需要
Nginx 1.18.0  
MySQL 8.0.36  
Redis 7.2.4   
muduo网络库  

# Nginx 负载均衡配置
```
stream {
    upstream MyServer {
	    server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s;
	    server 127.0.0.1:6002 weight=1 max_fails=3 fail_timeout=30s;
    }
    server{
	    proxy_connect_timeout 1s;
	    listen 8000;
	    proxy_pass MyServer;
	    tcp_nodelay on;
    }
}
```
可自行更改端口号、添加多台服务器

# MySQL
DML在目录bin/mysql.txt中
