#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unordered_map>
#include <functional>

#include "json.hpp"
#include "user.h"
#include "message.h"
#include "group.h"

using std::string;
using std::vector;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::unordered_map;
using std::function;
using json = nlohmann::json;

User user;                                  //当前用户
vector<User> myfriends;                     //好友
vector<Group> mygroups;                     //群组
bool isMainMenuRunning = false;

void showCurrentUserData();                 //显示当前用户信息
void readTaskHandler(int cfd);              //接收线程
void mainMenu(int cfd);                     //显示主菜单
string getCurrentTime();
void help(int cfd = -1, string cmd = "");
void chat(int cfd, string cmd);
void addfriend(int cfd, string cmd);
void creategroup(int cfd, string cmd);
void addgroup(int cfd, string cmd);
void groupchat(int cfd, string cmd);
void logout(int cfd, string cmd = "");

unordered_map<string, function<void(int ,string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addf", addfriend},
    {"cgrp", creategroup},
    {"addg", addgroup},
    {"gcht", groupchat},
    {"lout", logout}
};

unordered_map<string, string> commandMap = {
    {"help", "[show all command] help"},
    {"chat", "[chat with one friend] chat:<friendid>:<message>"},
    {"addf", "[add friend] addf:<friendid>"},
    {"cgrp", "[create group] cgrp:<groupname>:<groupdesc>"},
    {"addg", "[add group] addg:<groupid>"},
    {"gcht", "[group chat] gcht:<groupid>:<msg>"},
    {"lout", "[logout] lout"}
};

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        cerr << "Need three arguments: ./ChatClient 127.0.0.1 8888" << endl;
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //连接服务器
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cfd == -1)
    {
        cerr << "socket() error" << endl;
        exit(-1);
    }
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    if(connect(cfd, (sockaddr *)& server, sizeof(sockaddr_in)) == -1)
    {
        cerr << "connect() error" << endl;
        close(cfd);
        exit(-1);
    }

    while(1)
    {
        cout << "1. login \n2. register \n3. quit" << endl;
        cout << "your choice: ";
        int choice;
        cin >> choice;
        cin.get();
        switch(choice)
        {
            case 1:
            {
                int id = 0;
                string pwd;
                cout << "userid: ";
                cin >> id;
                cin.ignore();
                cout << "password: ";
                getline(cin, pwd);

                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["pwd"] = pwd;
                string req = js.dump();
                int len = send(cfd, req.c_str(), strlen(req.c_str()) + 1, 0);
                if(len == -1)
                {
                    cerr << "send() error: " << req << endl;
                }
                else
                {
                    char buf[1024];
                    len = recv(cfd, buf, 1024, 0);

                    //cout << buf << endl << endl;

                    if(len == -1)
                    {
                        cerr << "recv() error: " << req << endl;
                    }
                    else
                    {
                        json response = json::parse(buf);
                        if(response["errno"].get<int>() != 0)
                        {
                            cerr << response["errmsg"] << endl;
                        }
                        else
                        {
                            //记录用户信息
                            user.setId(response["id"].get<int>());
                            user.setName(response["name"]);

                            //记录好友列表
                            if(response.contains("friends"))
                            {
                                vector<string> friends = response["friends"];
                                for(string &str : friends)
                                {
                                    json frjs = json::parse(str);
                                    //User u;
                                    //u.setId(js["id"].get<int>());
                                    //u.setName(js["name"]);
                                    //u.setState(js["state"]);
                                    myfriends.emplace_back(frjs["id"].get<int>(), frjs["name"], "", frjs["state"]);
                                }
                            }
                            
                            //记录群组信息
                            if(response.contains("groups"))
                            {
                                vector<string> groups = response["groups"];
                                for(string &str : groups)
                                {
                                    json groupjs = json::parse(str);
                                    Group group;
                                    group.setId(groupjs["id"].get<int>());
                                    group.setName(groupjs["groupname"]);
                                    group.setDesc(groupjs["groupdesc"]);

                                    vector<string> groupusers = groupjs["users"];
                                    for(string &groupuser : groupusers)
                                    {
                                        GroupUser guser;
                                        json gujs = json::parse(groupuser);
                                        guser.setId(gujs["id"].get<int>());
                                        guser.setName(gujs["name"].get<string>());
                                        guser.setState(gujs["state"]);
                                        guser.setRole(gujs["role"]);
                                        group.getUsers().push_back(guser);
                                    }
                                    mygroups.push_back(group);
                                }
                            }
                            
                            //显示用户基本信息
                            showCurrentUserData();

                            //显示离线消息
                            if(response.contains("offlinemsg"))
                            {
                                vector<string> msgs = response["offlinemsg"];
                                for(string &str : msgs)
                                {
                                    json msgjs = json::parse(str);
                                    //cout << msgjs["time"] << " " << msgjs["id"] << " " << msgjs["name"]
                                    //     << " : " << msgjs["msg"] << endl;
                                    int msgtype = msgjs["msgid"].get<int>();
                                    if (msgtype == CHAT_MSG)
                                    {
                                        cout << msgjs["time"].get<string>() << " " << msgjs["id"].get<int>() << " "
                                             << msgjs["name"].get<string>() << " : " << msgjs["msg"].get<string>() << endl;
                                    }
                                    else if (msgtype == GROUP_CHAT_MSG)
                                    {
                                        cout << "[groupMessage] " << msgjs["groupid"] << " " << msgjs["time"].get<string>() << " "
                                             << msgjs["id"].get<int>() << " " << msgjs["name"].get<string>() << " : "
                                             << msgjs["msg"].get<string>() << endl;
                                    }
                                }
                            }

                            //启动线程接收数据
                            static int threadnum = 0;
                            if(threadnum == 0)
                            {
                                std::thread readTask(readTaskHandler, cfd);
                                readTask.detach();
                                ++threadnum;
                            }

                            //进入聊天主页面
                            isMainMenuRunning = true;
                            mainMenu(cfd);
                        }
                    }
                }
                break;
            }
            case 2:
            {
                char name[50];
                char pwd[50];
                bzero(name, sizeof(name));
                bzero(pwd, sizeof(pwd));

                cout << "username: ";
                cin.getline(name, 50);
                cout << "password: ";
                cin.getline(pwd, 50);

                json js;
                js["msgid"] = REGIST_MSG;
                js["name"] = name;
                js["pwd"] = pwd;
                string req = js.dump();

                int len = send(cfd, req.c_str(), strlen(req.c_str()) + 1, 0);
                if(len == -1)
                {
                    cerr << "send() error: " << req << endl;
                }
                else
                {
                    char buf[1024];
                    bzero(buf, sizeof(buf));
                    len = recv(cfd, buf, 1024, 0);
                    if(len == -1)
                    {
                        cerr << "recv() error" << endl;
                    }
                    else
                    {
                        json response = json::parse(buf);
                        if(response["errno"].get<int>() != 0)
                        {
                            cerr << name << "is already exists" << endl;
                        }
                        else
                        {
                            cout << name << " regist success, uid is " << response["id"] << endl;
                        }
                    }
                }
                break;
            }
            case 3:
            {
                exit(0);
            }
            default:
            {
                cerr << "invalid input" << endl;
                break;
            }
        }
    }
}

void showCurrentUserData()
{
    cout << "=== CURRENT USER INFO ===" << endl;
    cout << "id: " << user.getId() <<"\tname: " << user.getName() << endl;
    cout << "--friends--" << endl;
    if(!myfriends.empty())
    {
        for(User &u : myfriends)
        {
            cout << u.getId() << "\t" << u.getName() << "\t" << u.getState() << endl; 
        }
    }
    cout << "--groups--" << endl;
    if(!mygroups.empty())
    {
        for(Group &g : mygroups)
        {
            cout << g.getId() << "\t" << g.getName() << "\t" << g.getDesc() << endl; 
            for(GroupUser &u : g.getUsers())
            {
            cout << u.getId() << "\t" << u.getName() << "\t" << u.getState() << "\t" << u.getRole() << endl; 
            }
        }
    }
    cout << "=== ================= ===" << endl;
}

void readTaskHandler(int cfd)
{
    while(1)
    {
        char buf[1024];
        memset(buf, 0, 1024);
        int len = recv(cfd, buf, 1024, 0);
        if(len == -1 || len == 0)
        {
            close(cfd);
            exit(-1);
        }

        json js = json::parse(buf);
        int msgtype = js["msgid"].get<int>();
        if(msgtype == CHAT_MSG)
        {
            cout << js["time"].get<string>() << " " << js["id"].get<int>() << " "
                << js["name"].get<string>() << " : " << js["msg"].get<string>() << endl;
            continue;
        }
        else if(msgtype == GROUP_CHAT_MSG)
        {
            cout << "[groupMessage: ]"<< js["groupid"] << " " << js["time"].get<string>() << " " 
                << js["id"].get<int>() << " " << js["name"].get<string>() << " : " 
                << js["msg"].get<string>() << endl;
            continue;
        }
    }
}

void mainMenu(int cfd)
{
    help();

    string buf;
    while(isMainMenuRunning)
    {
        getline(cin, buf);
        string commandbuf(buf);
        string command;
        //cout << "buf " << buf << endl;

        //为了兼容有参数命令和无参数命令
        int idx = commandbuf.find(":");
        command = idx == -1 ? commandbuf : commandbuf.substr(0, idx);
        //cout << "command " << command << endl;

        auto it = commandHandlerMap.find(command);
        if(it == commandHandlerMap.end())
        {
            cerr << "invalid command" << endl;
            continue;
        }
        it->second(cfd, commandbuf.substr((idx + 1), commandbuf.size() - idx));
    }
}

string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt); 
    char ch[80];
    sprintf(ch, "%d:%d:%d %d:%d:%d", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    return string(ch);
}

void help(int cfd, string cmd)
{
    cout << "command list" << endl;
    for(auto &it : commandMap)
    {
        cout << it.first << " : " << it.second << endl;
    }
    cout << endl;
}

void chat(int cfd, string cmd)
{
    int idx = cmd.find(":");
    if(idx == -1)
    {
        cerr << "chat command invalid" << endl;
        return;
    }

    int friendid = atoi(cmd.substr(0, idx).c_str());
    string msg = cmd.substr(idx + 1, cmd.size() - idx);

    json js;
    js["msgid"] = CHAT_MSG;
    js["id"] = user.getId();
    js["name"] = user.getName();
    js["to"] = friendid;
    js["time"] = getCurrentTime();
    js["msg"] = msg;
    string buf = js.dump();

    int len = send(cfd, buf.c_str(), strlen(buf.c_str()) + 1, 0);
    if(len == -1)
    {
        cerr << "send error : chat to one friend : " << buf << endl;
    }
}

void addfriend(int cfd, string cmd)
{
    int friendid = atoi(cmd.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = user.getId();
    js["friendid"] = friendid;
    string buf = js.dump();

    int len = send(cfd, buf.c_str(), strlen(buf.c_str()) + 1, 0);
    if(len == -1)
    {
        cerr << "send() error : in addfriend : " << buf << endl; 
    }
}

void creategroup(int cfd, string cmd)
{
    int idx = cmd.find(":");
    if(idx == -1)
    {
        cerr << "chat command invalid" << endl;
        return;
    }

    string groupname = cmd.substr(0, idx);
    string groupdesc = cmd.substr(idx + 1, cmd.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = user.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buf = js.dump();

    int len = send(cfd, buf.c_str(), strlen(buf.c_str()) + 1, 0);
    if(len == -1)
    {
        cerr << "send() error : create group : " << buf << endl;
    }
}

void addgroup(int cfd, string cmd)
{
    int groupid = atoi(cmd.c_str());

    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = user.getId();
    js["groupid"] = groupid;
    string buf = js.dump();

    int len = send(cfd, buf.c_str(), strlen(buf.c_str()) + 1, 0);
    if(len == -1)
    {
        cerr << "send() error : add group : " << buf << endl;
    }
}

void groupchat(int cfd, string cmd)
{
    int idx = cmd.find(":");
    if(idx == -1)
    {
        cerr << "chat command invalid" << endl;
        return;
    }

    int groupid = atoi(cmd.substr(0, idx).c_str());
    string msg = cmd.substr(idx + 1, cmd.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = user.getId();
    js["name"] = user.getName();
    js["groupid"] = groupid;
    js["msg"] = msg;
    js["time"] = getCurrentTime();
    string buf = js.dump();

    int len = send(cfd, buf.c_str(), strlen(buf.c_str()) + 1, 0);
    if(len == -1)
    {
        cerr << "send() error : chat group : " << buf << endl;
    }
}

void logout(int cfd, string cmd)
{
    json js;
    js["msgid"] = LOGOUT_MSG;
    js["id"] = user.getId();
    string buf = js.dump();

    int len = send(cfd, buf.c_str(), strlen(buf.c_str()) + 1, 0);
    if(len == -1)
    {
        cerr << "send() error : logout : " << buf << endl;
    }
    isMainMenuRunning = false;
    mygroups.clear();
    myfriends.clear();
}