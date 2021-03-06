#ifndef OGGTUBE_OGGTUBE_H
#define OGGTUBE_OGGTUBE_H

#include <string>
#include <vector>

class Oggtube {

    const size_t npos = std::string::npos;

    std::string buffer;

public:
    void download(size_t pos);

    bool parse(const std::string& str, const std::string& itag=":249");

    inline std::string* getBufferPtr()  
    {
        return buffer.empty() ? nullptr : &buffer;
    }
};


#endif //OGGTUBE_OGGTUBE_H
