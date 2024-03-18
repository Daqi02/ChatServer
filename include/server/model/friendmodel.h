#pragma once

#include <vector>
#include "friend.h"
#include "user.h"
#include "mysqldb.h"

using std::vector;

class FriendModel
{
public:
    bool insert(Friend user);
    bool remove(Friend user);
    vector<User> query(int uid);

private:
};