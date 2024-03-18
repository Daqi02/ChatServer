# ChatServer
基于muduo库并实现Nginx负载均衡的集群聊天服务器ChatServer

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
