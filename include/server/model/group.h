#pragma once

#include "groupuser.h"
#include <string>
#include <vector>

using std::string;
using std::vector;

class Group
{
    using cstring = const string;

public:
    Group(int _id = -1, cstring& _name = "", cstring& _descript = "")
        :id(_id), name(_name), desc(_descript)
    {
    }

    void setId(int _id) { id = _id; }
    void setName(cstring& _name) { name = _name; }
    void setDesc(cstring&  _descript) { desc = _descript; }

    int getId() { return id; }
    string getName() { return name; }
    string getDesc() { return desc; }
    vector<GroupUser> &getUsers() { return users; }

private:
    int id;
    string name;
    string desc;
    vector<GroupUser> users;
};