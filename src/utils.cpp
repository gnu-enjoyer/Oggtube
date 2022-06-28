#include "utils.h"
#include "fmt/format.h"
#include "fmt/chrono.h"
#include <ctime>

void Logger::write(const std::string& str, bool err ) const {

    std::scoped_lock lock(mtx);

    std::ofstream logfile("log.txt", std::ofstream::app);

    logfile << fmt::format("{0:%F_%T}", std::chrono::system_clock::now());

    err ? logfile << " [ERROR] " << str << std::endl : logfile << " [INFO] " << str << std::endl;

}  