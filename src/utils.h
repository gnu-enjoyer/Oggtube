#ifndef OGGTUBE_UTILS_H
#define OGGTUBE_UTILS_H

#include <string>
#include <iosfwd>
#include <fstream>
#include <mutex>

class Logger {

    mutable std::mutex mtx;

public:   
    void write(const std::string& str, bool err = false) const;

};

inline Logger const Log;

#endif //OGGTUBE_UTILS_H
