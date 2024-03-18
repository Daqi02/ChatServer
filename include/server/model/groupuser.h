#pragma once

#include <string>
#include "user.h"

using std::string;

class GroupUser : public User
{

public:
    GroupUser(int _id, string _name, string _state, string _role)
        :User(_id, _name, "", _state), role(_role)
    {
    }

    GroupUser()
    {
    }

    void setRole(cstring& _role) { this->role = _role; }
    string getRole() { return this->role; }

private:
    string role;
};