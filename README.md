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
```  
