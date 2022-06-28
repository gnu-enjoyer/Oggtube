#include "utils.h"
#include "oggtube.h"
#include "simdjson.h"
#include "decipher.h"
#include <iosfwd>

void Oggtube::download(size_t pos){

    size_t endpos = buffer.find("},{", pos); 

    std::string json = "{\"itag\"";

    for (pos; pos < endpos; ++pos)
        json.push_back(buffer.at(pos));    

    json.append("}");

    simdjson::ondemand::parser parser;

    simdjson::padded_string padded_json{json};

    simdjson::ondemand::document itag = parser.iterate(padded_json);

    std::stringstream js;

    if(json.find("signatureCipher") == npos) {
        js << itag["url"].get_string();
        buffer = js.view();
        return;
    }

    std::string extracted_cipher;

    js << itag["signatureCipher"];

    for(auto &c : js.view()){
        if( c == '\\')
            break;

        extracted_cipher.push_back(c);
    }

    auto url = js.view().substr((js.view().find("s://"))-4, js.view().size());

    extracted_cipher.erase(0, 3);

    Decipher Crypto = Decipher::Instance(buffer);

    Crypto.DecipherSignature(&extracted_cipher);

    buffer = url;

    buffer.pop_back();

    buffer.append("&sig=" + extracted_cipher);
}

bool Oggtube::parse(const char* in) {

    std::string path = "/watch?v=";

    path.append(in);

    Utils::yt_to_string(path.c_str(), buffer);

    size_t pos = buffer.find(":249"); //ytInitialPlayerResponse for all itags, later

    if(pos != npos)
        download(pos);

    return(pos != npos);
}
