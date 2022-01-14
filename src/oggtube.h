#ifndef OGGTUBE_OGGTUBE_H
#define OGGTUBE_OGGTUBE_H

#include <string>
#include <vector>

class Oggtube {

    const size_t npos = std::string::npos;
    std::string buffer;

public:

    bool parse(const char* in);
    std::string* getBufferPtr();

    Oggtube();
    ~Oggtube() = default;

};


#endif //OGGTUBE_OGGTUBE_H
