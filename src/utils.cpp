#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_ZLIB_SUPPORT
#define CPPHTTPLIB_THREAD_POOL_COUNT 1
#include "httplib.h"
#include "utils.h"
#include <ctime>

void Logger::write(const char *in, bool err) {

    std::time_t time = std::time(0);
    *logfile << std::asctime(std::localtime(&time));

    err ? *logfile << " [ERROR] " << in << std::endl : *logfile << " [INFO] " << in << std::endl;

}

bool Utils::yt_to_string(const char *in, std::string &buff) {

        httplib::Headers headers = {
                {"User-Agent",      "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:95.0) Gecko/20100101 Firefox/95.0"},
                {"Accept-Encoding", "gzip"},
                {"Keep-Alive",      "0"}
        };


        httplib::Client cli("https://www.youtube.com");

        cli.set_keep_alive(false);

        if(auto res = cli.Get(in, headers)){

            if (res->status != 200) return false;

            buff = res->body;

            return true;

        }else{

            Logger::get().write("[Utils] Network error.", true);
            return false;

        }

}

void Utils::HTMLtoUTF8(std::string &scraped) {

    //TBD: refactor
    replaceAll(scraped, "%25", "%");
    replaceAll(scraped, "%26", "&");
    replaceAll(scraped, "%2C", ",");
    replaceAll(scraped, "%2F", "/");
    replaceAll(scraped, "%3A", ":");
    replaceAll(scraped, "%3D", "=");
    replaceAll(scraped, "%3F", "?");

}

void Utils::replaceAll(std::string &source, const std::string_view &from, const std::string_view &to) {
    std::string newString;
    newString.reserve(source.length());

    std::string::size_type lastPos = 0;
    std::string::size_type findPos;

    while (std::string::npos != (findPos = source.find(from, lastPos))) {
        newString.append(source, lastPos, findPos - lastPos);
        newString += to;
        lastPos = findPos + from.length();
    }

    newString += source.substr(lastPos);

    source.swap(newString);

}
