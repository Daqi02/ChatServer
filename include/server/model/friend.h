#pragma once

#include <string>

using std::string;

class Friend
{
public:
    Friend(int _id = -1, int _fid = -1)
        :userid(_id), friendid(_fid)
    {
    }

    void setId(int _id) { this->userid = _id; }
    void setFirendId(int _id) { this->friendid = _id; }
    
    int getUserId() { return userid; }
    int getFriendId() { return friendid; }

private:
    int userid;
    int friendid;
};