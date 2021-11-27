#include "rest.h"
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
//#include <cpprest/details/basic_types.h>


/*
 * C++ Rest, static
 */

void Oggtube::http_as_string(std::string *toSet, std::string inString) {
    auto in_url = utility::conversions::to_string_t(inString);
    auto fileStream = std::make_shared<Concurrency::streams::ostream>();

    pplx::task<void> requestTask = Concurrency::streams::fstream::open_ostream(U("debug.html")).then(
                    [=](Concurrency::streams::ostream outFile) {
                        *fileStream = outFile;
                        web::http::client::http_client client(U("https://www.youtube.com/"));
                        return client.request(web::http::methods::GET, in_url);
                    })

            .then([=](web::http::http_response response) {
                printf("Received response status code:%u\n", response.status_code());

                Concurrency::streams::stringstreambuf buffer;
                response.body().read_to_end(buffer).get();
                toSet->append(buffer.collection());

                return fileStream->print(buffer.collection());
            })

            .then([=](size_t) {
                return fileStream->close();
            });

    try {
        requestTask.wait();
    }
    catch (const std::exception &e) {
        printf("Error exception:%s\n", e.what());
    }

    //return true;


}