#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_ZLIB_SUPPORT
#define CPPHTTPLIB_THREAD_POOL_COUNT 4

#include "router.h"
#include "muxxer.h"
#include "oggtube.h"
#include "httplib.h"

const size_t DATA_CHUNK_SIZE = 4;

void Router::listen(int port){

  httplib::Server svr; 

  svr.Get("/test", [](const httplib::Request &req, httplib::Response &res) {

    //if(!req.has_param("key")) return;

    //todo sanity checks/
    
    auto val = req.get_param_value("key");    

    auto oggt = Oggtube();      

    if(!oggt.parse(val)) return;

    std::string outp = val + ".ogg";

    auto data = new std::string(*oggt.getBufferPtr());

  res.set_content_provider(
    data->size(), // Content length
    "text/plain", // Content type
    [data](size_t offset, size_t length, httplib::DataSink &sink) {
      const auto &d = *data;
      sink.write(&d[offset], std::min(length, DATA_CHUNK_SIZE));
      return true; // return 'false' if you want to cancel the process.
    },
    [data](bool success) { delete data; });

  });  

  svr.listen("0.0.0.0", port);

}

bool Parser::yt_to_string(const std::string& str, std::string &buff) {

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
