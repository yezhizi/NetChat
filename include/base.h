#ifndef _BASE_H
#define _BASE_H
#include <string>
namespace ntc
{
    static const int kvEmpty = 0;

    static const std::string kServerIP = "127.0.0.1";
    static const int kServerPort = 8081;

    static const int ServerID = 1;


    struct User
    {
        int id;
        std::string name;
        std::string phone_number;
        std::string password;
    };
}



#endif //_BASE_H