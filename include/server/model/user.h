#pragma once

#include <string>

using std::string;

using cstring = const string;

class User
{

public:
    User(int _id = -1, string _name = "", string _password = "", string _state = "offline")
        :id(_id), name(_name), password(_password), state(_state)
    {
    }

    void setId(int _id) { this->id = _id; }
    void setPassword(cstring& _pwd) { this->password = _pwd; }
    void setName(cstring& _user) { this->name = _user; }
    void setState(cstring& _state) { this->state = _state; }
    
    int getId() const { return id; }
    string getPassword() const { return password; }
    string getName() const { return name; }
    string getState() const { return state; }

private:
    int id;
    string name;
    string password;
    string state;
};