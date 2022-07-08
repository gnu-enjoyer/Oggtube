#ifndef OGGTUBE_ROUTER_H
#define OGGTUBE_ROUTER_H

#include <optional>
#include <string>

class Router{

public:
    void listen(int port);

};

class Parser {

public:
    static std::optional<std::string> yt_to_string(const std::string& str);

};


#endif //OGGTUBE_ROUTER_H
