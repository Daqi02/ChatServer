#pragma once

#include <string>
#include <string.h>
#include <stdio.h>
#include <vector>
#include "mysqldb.h"
#include "offlinemessage.h"

using std::string;
using std::vector;

class OfflineMessageModel
{
public:
    bool insert(OfflineMessage& offmsg);
    vector<string> query(int id);
    void remove(int id);

private:
    
};