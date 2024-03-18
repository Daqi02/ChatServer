#pragma once

#include <string>

using std::string;

class OfflineMessage
{
    using cstring = const string;

public:
    OfflineMessage(int _id = -1, cstring& _msg = "")
        :id(_id), message(_msg)
    {
    }

    void setId(int _id) { this->id = _id; }
    void setMessage(cstring& _msg) { this->message = _msg; }
    
    int getId() { return id; }
    string getMessage() { return message; }

private:
    int id;
    string message;
};