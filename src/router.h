#ifndef OGGTUBE_ROUTER_H
#define OGGTUBE_ROUTER_H

#include <string>

class Router{

public:
    void listen(int port);

};

class Parser {

public:
    static bool yt_to_string(const std::string& str, std::string &buff);

};


#endif //OGGTUBE_ROUTER_H
