#ifndef OGGTUBE_UTILS_H
#define OGGTUBE_UTILS_H

#include <string>
#include <iosfwd>
#include <fstream>

class Logger {

    //Meyers Singleton
    Logger() = default;

    ~Logger() = default;

    Logger(const Logger &) = delete;

    Logger &operator=(const Logger &) = delete;

    std::ofstream *logfile = new std::ofstream("log.txt",
                                               std::fstream::app);

public:

    static Logger &get() {
        static Logger instance;
        // volatile int dummy{};
        return instance;
    }

    void write(const char *in, bool err = false);


};

class Utils {

public:

    static void replaceAll(std::string &source, const std::string_view &from, const std::string_view &to);

    static void HTMLtoUTF8(std::string &scraped);

    static bool yt_to_string(const char *in, std::string &buff);

};


#endif //OGGTUBE_UTILS_H
