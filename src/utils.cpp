#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_ZLIB_SUPPORT
#define CPPHTTPLIB_THREAD_POOL_COUNT 1

#include "httplib.h"
#include "utils.h"
#include "fmt/format.h"
#include "fmt/chrono.h"
#include <ctime>

#include <locale>

void Logger::write(const std::string& str, bool err ) const {

    std::scoped_lock lock(mtx);

    std::ofstream logfile("log.txt", std::ofstream::app);

    logfile << fmt::format("{0:%F_%T}", std::chrono::system_clock::now());

    err ? logfile << " [ERROR] " << str << std::endl : logfile << " [INFO] " << str << std::endl;

}  

bool Utils::yt_to_string(const std::string& str, std::string &buff) {

    httplib::Headers headers = {
                {"User-Agent",      "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:95.0) Gecko/20100101 Firefox/95.0"},
                {"Accept-Encoding", "gzip"},
                {"Keep-Alive",      "0"}
    };

    httplib::Client cli("https://www.youtube.com");

    cli.set_keep_alive(false);

    auto res = cli.Get(str.c_str(), headers);    

    if(!res || res->status != 200) return false;

    buff = res->body;

    return res->status == 200;
}
