#include "oggtube.hpp"

#include "oggtube.h"
#include "demux.h"
#include "nlohmann/json.hpp"
#include "rest.h"

#include <regex>
#include <sstream>
#include <re2/re2.h>


/*
 *  Global precompiled RE2
 */

static const Oggtube::regexes rgx({
                                  new RE2(R"(\b[cs]\s*&&\s*[adf]\.set\([^,]+\s*,\s*encodeURIComponent\s*\(\s*([a-zA-Z0-9$]+)\()"),
                                  new RE2(R"(\b[a-zA-Z0-9]+\s*&&\s*[a-zA-Z0-9]+\.set\([^,]+\s*,\s*encodeURIComponent\s*\(\s*([a-zA-Z0-9$]+)\()"),
                                  new RE2(R"(\bm=([a-zA-Z0-9$]{2})\(decodeURIComponent\(h\.s\)\))"),
                                  new RE2(R"(\bc&&\(c=([a-zA-Z0-9$]{2})\(decodeURIComponent\(c\)\))"),
                                  new RE2(R"((?:\b|[^a-zA-Z0-9$])([a-zA-Z0-9$]{2})\s*=\s*function\(\s*a\s*\)\s*\{\s*a\s*=\s*a\.split\(\s*""\s*\);[a-zA-Z0-9$]{2}\.[a-zA-Z0-9$]{2}\(a,\d+\))"),
                                  new RE2(R"((?:\b|[^a-zA-Z0-9$])([a-zA-Z0-9$]{2})\s*=\s*function\(\s*a\s*\)\s*\{\s*a\s*=\s*a\.split\(\s*""\s*\))"),
                                  new RE2(R"(([a-zA-Z0-9$]+)\s*=\s*function\(\s*a\s*\)\s*\{\s*a\s*=\s*a\.split\(\s*""\s*\))")
                          });

static const std::regex yt_reg(
        R"(^((?:https?:)?\/\/)?((?:www|m)\.)?((?:youtube\.com|youtu.be))(\/(?:[\w\-]+\?v=|embed\/|v\/)?)([\w\-]+)(\S+)?$)");

/*
 *  Static utils
 */

int Oggtube::parse_input(std::string &in) {
    if (in.empty()) return 0;
    if (in.length() == 11) return 1;


    std::smatch m;
    std::regex_search(in, m, yt_reg);

    if (m[5].length() == 11) {
        in = m[5];
        return 1;
    }


    return 0;
}

void Oggtube::replaceAll(std::string &source, const std::string_view &from, const std::string_view &to) {
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

inline void Oggtube::HTMLtoUTF8(std::string &scraped){

    //TBD: refactor
    Oggtube::replaceAll(scraped, "%25", "%");
    Oggtube::replaceAll(scraped, "%26", "&");
    Oggtube::replaceAll(scraped, "%2C", ",");
    Oggtube::replaceAll(scraped, "%2F", "/");
    Oggtube::replaceAll(scraped, "%3A", ":");
    Oggtube::replaceAll(scraped, "%3D", "=");
    Oggtube::replaceAll(scraped, "%3F", "?");
}

/*
 * API calls
 */

bool OggtubeAPI::startOggtube() {
    if(rgx.re_list.empty()){return false;}else return true;
}

int OggtubeAPI::download(const char string[12]) {
    if(!OggtubeAPI::startOggtube()) return false;

    std::string input = "/watch?v=";
    std::string html_response;

    Oggtube::http_as_string(&html_response, input.append(string));

    if (const std::size_t spos = html_response.find(":249"); spos != std::string::npos) {
        std::string scraped = "{";
        int lol = 1;

        for (int i = (spos + 5); i < spos + 2900; i++) {
            if (lol != 0) {
                char c = html_response.at(i);
                scraped += c;
                if (c == '{') {
                    lol = lol + 1;
                } else if (c == '}') {
                    lol = lol - 1;
                } //funniest shit i ever seen
            }
        }


        Oggtube::HTMLtoUTF8(scraped);
        nlohmann::json js_itag = nlohmann::json::parse(scraped);

        if (js_itag["signatureCipher"].empty()) {
            scraped = js_itag["url"].dump();
            scraped.pop_back();
            scraped.erase((scraped.begin()));
            goto tmux;
        } else {

            auto final_str_builder = js_itag["signatureCipher"].dump();
            std::string extracted_cipher;

            for (const auto &c: final_str_builder) {
                if (c != '&')
                    extracted_cipher.push_back(c);
                else break;

            }
            extracted_cipher.erase(0, 3);
            final_str_builder.erase(0, extracted_cipher.size() + 15);
            final_str_builder.pop_back();

            /* Decrypt */
            kloeckner::Decipher Crypto = kloeckner::Decipher::Instance(html_response);
            Crypto.DecipherSignature(&extracted_cipher);

            if (!extracted_cipher.empty()) {
                scraped = final_str_builder + "&sig=" + extracted_cipher;
                //extracted_cipher.erase(0, 1); //?
                goto tmux;
            } else
                return 2;
        }

        tmux:
        Oggtube::muxxer muxxy;
        muxxy.transmux(scraped.c_str(), "debug.ogg");
        return 0;

    }

    //Network error handling here
    return 1;
}

/*
 *  Decrypto
 */

std::string kloeckner::Decipher::LoadDecipherFuncName(const std::string &p_decipher_js) {
    int x = 0;
    re2::StringPiece y;
    re2::StringPiece z;


    for (auto &e: rgx.re_list) {
        if (static_cast<RE2*>(e)->ok()) {
            auto found = static_cast<RE2*>(e)->Match(p_decipher_js, 0, p_decipher_js.size(), RE2::UNANCHORED, &y, x);

            if (found) {
                RE2::PartialMatch(p_decipher_js, *static_cast<RE2*>(e), &z);
                return z.ToString();
            }
        }
    }

    throw std::runtime_error("Could not find decipher function name!");
}


/*
 * Code below is copyright (c) 2018 Linus Kloeckner
*/

void kloeckner::Decipher::DecipherSignature(std::string *p_signature) {
    for (const auto &sub: m_decipher) {
        const std::string &p_func_name = std::get<0>(sub);
        if (p_func_name == m_sub_reverse_name) {
            SubReverse(p_signature);
        } else if (p_func_name == m_sub_splice_name) {
            SubSplice(p_signature, std::get<1>(sub));
        } else if (p_func_name == m_sub_swap_name) {
            SubSwap(p_signature, std::get<1>(sub));
        }
    }
}

void kloeckner::Decipher::LoadDecipher(const std::string &p_video_html) {
    std::string decipher_js = LoadDecipherJS(p_video_html);
    std::string decipher_func_name = LoadDecipherFuncName(decipher_js);
    std::string decipher_func_definition = LoadDecipherFuncDefinition(decipher_js, decipher_func_name);
    std::string sub_func_name = LoadSubFuncName(decipher_func_definition);
    std::string sub_func_definition = LoadSubFuncDefinition(decipher_js, sub_func_name);
    ExtractSubFuncNames(sub_func_definition);
    ExtractDecipher(decipher_func_definition);
}

std::string kloeckner::Decipher::LoadDecipherJS(const std::string &p_video_html) {
    //TBD: Move to RE2
    std::regex expr_player_url(R"((?:PLAYER_JS_URL|jsUrl)\"\s*:\s*\"([^"]+))");
    std::smatch matches_player_url;
    std::regex_search(p_video_html, matches_player_url, expr_player_url);

    if (matches_player_url.empty()) {
        throw std::runtime_error("Could not find player URL!");
    }

    std::string str_decipher = matches_player_url[1];

    std::string str_body;
    Oggtube::http_as_string(&str_body, str_decipher);

    return str_body;
}


std::string kloeckner::Decipher::LoadDecipherFuncDefinition(const std::string &p_decipher_js,
                                                            const std::string &p_decipher_func_name) {

    //TBD: Move to RE2
    std::regex expr_sub_func_definition(p_decipher_func_name + R"(=function\(.+?\)\{(.+?)\})");
    std::smatch matches_sub_funcs;

    std::regex_search(p_decipher_js, matches_sub_funcs, expr_sub_func_definition);
    return matches_sub_funcs[1];
}

std::string kloeckner::Decipher::LoadSubFuncName(const std::string &p_decipher_func_definition) {

    //TBD: Move to RE2
    std::regex expr_sub_func_name(R"((..)\.(..)\(.,(\d)+\))");
    std::smatch matches_sub_func;

    std::stringstream ss(p_decipher_func_definition);
    std::string item;
    while (std::getline(ss, item, ';')) {
        std::regex_search(item, matches_sub_func, expr_sub_func_name);
        std::string str_sub_func_name = matches_sub_func[1];

        if (!str_sub_func_name.empty())
            return str_sub_func_name;
    }

    return "ERROR";
}

std::string kloeckner::Decipher::LoadSubFuncDefinition(const std::string &p_decipher_js,
                                                       const std::string &p_sub_func_name) {
    std::string fixed_sub_func_name(p_sub_func_name);
    std::string special_chars = "@&+";   // TODO: add possible chars used by youtube
                                            // m:    - is used in some urls

    for (auto c: special_chars) {
        auto pos = fixed_sub_func_name.find(c);
        if (pos != std::string::npos) {
            std::string fixed_char("\\");
            fixed_char.append(1, c);
            fixed_sub_func_name.replace(pos, 1, fixed_char);
        }
    }
    //TBD: Move to RE2
    std::regex expr_sub_func_definition(R"(var\s)" + fixed_sub_func_name + R"(=\{((?:\n|.)*?)\};)");
    std::smatch matches_sub_func_definitions;
    std::regex_search(p_decipher_js, matches_sub_func_definitions, expr_sub_func_definition);

    return matches_sub_func_definitions[1];
}

void kloeckner::Decipher::ExtractSubFuncNames(const std::string &p_sub_func_definition) {

    //TBD: Move to RE2
    std::regex expr_sub_func_names(R"((\w\w):function\(.+?\)\{(.*?)\})");
    std::sregex_iterator iter_end;
    std::sregex_iterator iter_sub_func_names(p_sub_func_definition.begin(), p_sub_func_definition.end(),
                                             expr_sub_func_names);

    while (iter_sub_func_names != iter_end) {
        std::smatch matches_sub_func_name = *iter_sub_func_names++;

        std::string str_def = matches_sub_func_name[2];

        if (str_def.find("reverse") != std::string::npos)
            m_sub_reverse_name = matches_sub_func_name[1];
        else if (str_def.find("splice") != std::string::npos)
            m_sub_splice_name = matches_sub_func_name[1];
        else
            m_sub_swap_name = matches_sub_func_name[1];
    }
}

void kloeckner::Decipher::ExtractDecipher(const std::string &p_decipher_func_definition) {

    //TBD: Move to RE2
    std::regex expr_sub_func(R"(\.(..)\(.,(\d+)\))");
    std::smatch matches_sub_func;

    std::stringstream ss(p_decipher_func_definition);
    std::string item;
    while (std::getline(ss, item, ';')) {
        if (!std::regex_search(item, matches_sub_func, expr_sub_func))
            continue;

        std::string func_name = matches_sub_func[1];
        std::string func_arg_test = matches_sub_func[2];
        int func_arg = stoi(matches_sub_func[2]);

        m_decipher.emplace_back(func_name, func_arg);
    }
}

void kloeckner::Decipher::SubReverse(std::string *p_a) {
    std::reverse(p_a->begin(), p_a->end());
}

void kloeckner::Decipher::SubSplice(std::string *p_a, int p_b) {
    p_a->erase(0, static_cast<uint64_t>(p_b));
}

void kloeckner::Decipher::SubSwap(std::string *p_a, int p_b) {
    char c = (*p_a)[0];
    (*p_a)[0] = (*p_a)[p_b % p_a->length()];
    (*p_a)[p_b] = c;
}


