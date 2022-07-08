#include "utils.h"
#include "router.h"
#include "decipher.h"
#include "re2/re2.h"
#include "ctre.hpp"
#include <sstream>

static constexpr ctll::fixed_string expr = R"((\w\w):function\(.+?\)\{(.*?)\})";

Regexes* Decipher::AllocateRegex() {

    static Regexes rgx({
                               new RE2(R"(\b[cs]\s*&&\s*[adf]\.set\([^,]+\s*,\s*encodeURIComponent\s*\(\s*([a-zA-Z0-9$]+)\()"),
                               new RE2(R"(\b[a-zA-Z0-9]+\s*&&\s*[a-zA-Z0-9]+\.set\([^,]+\s*,\s*encodeURIComponent\s*\(\s*([a-zA-Z0-9$]+)\()"),
                               new RE2(R"(\bm=([a-zA-Z0-9$]{2})\(decodeURIComponent\(h\.s\)\))"),
                               new RE2(R"(\bc&&\(c=([a-zA-Z0-9$]{2})\(decodeURIComponent\(c\)\))"),
                               new RE2(R"((?:\b|[^a-zA-Z0-9$])([a-zA-Z0-9$]{2})\s*=\s*function\(\s*a\s*\)\s*\{\s*a\s*=\s*a\.split\(\s*""\s*\);[a-zA-Z0-9$]{2}\.[a-zA-Z0-9$]{2}\(a,\d+\))"),
                               new RE2(R"((?:\b|[^a-zA-Z0-9$])([a-zA-Z0-9$]{2})\s*=\s*function\(\s*a\s*\)\s*\{\s*a\s*=\s*a\.split\(\s*""\s*\))"),
                               new RE2(R"(([a-zA-Z0-9$]+)\s*=\s*function\(\s*a\s*\)\s*\{\s*a\s*=\s*a\.split\(\s*""\s*\))")
                       });

    return &rgx;
}

Regexes::~Regexes() {

    for (auto &r : re_list)
        delete(static_cast<RE2*>(r));    
}

std::string Decipher::LoadDecipherFuncName(const std::string &p_decipher_js) {

    int x = 0;

    re2::StringPiece y;

    re2::StringPiece z;

    for (auto &e: rPtr->re_list)
        if (static_cast<RE2*>(e)->ok())
            if (auto found = static_cast<RE2*>(e)->Match(p_decipher_js, 0, p_decipher_js.size(), RE2::UNANCHORED, &y, x))
                {
                    RE2::PartialMatch(p_decipher_js, *static_cast<RE2*>(e), &z);
                    return z.ToString();
                }           

    throw std::runtime_error("Could not find decipher function name!");
}

std::string Decipher::LoadDecipherJS(const std::string &p_video_html) {

    auto m = ctre::search<R"((?:PLAYER_JS_URL|jsUrl)\"\s*:\s*\"([^"]+))">(p_video_html);

    if(!m) throw std::runtime_error("Could not find player URL!");

    return Parser::yt_to_string(m.get<1>().to_string()).value_or("Error deciphering Javascript");
}

std::string Decipher::LoadDecipherFuncDefinition(const std::string &p_decipher_js,
                                                            const std::string &p_decipher_func_name) {

    auto result = re2::RE2(p_decipher_func_name + R"(=function\(.+?\)\{(.+?)\})");

    if(!result.ok()) throw std::runtime_error("Error constructing runtime regular expression");

    re2::StringPiece z;
    
    RE2::PartialMatch(p_decipher_js, result, &z);    

    return z.ToString();
}

std::string Decipher::LoadSubFuncName(const std::string &p_decipher_func_definition) {

    return ctre::match<R"((..)\.(..)\(.,(\d)+\))">(p_decipher_func_definition).to_string();
}

std::string Decipher::LoadSubFuncDefinition(const std::string &p_decipher_js,
                                                       const std::string &p_sub_func_name) {
    std::string fixed_sub_func_name(p_sub_func_name);
    std::string special_chars = "@&+";   // TODO: add possible chars used by youtube
    // m:    - is used in some urls

    std::string fixed_char("\\");

    for (auto c: special_chars)
        if(auto pos = fixed_sub_func_name.find(c); pos != std::string::npos)
        {
            fixed_char.append(1, c);

            fixed_sub_func_name.replace(pos, 1, fixed_char);     
        }
    
    auto result = re2::RE2(R"(var\s)" + fixed_sub_func_name + R"(=\{((?:\n|.)*?)\};)");

    re2::StringPiece z;
    
    RE2::PartialMatch(p_decipher_js, result, &z);  

    return z.ToString();
}

void Decipher::ExtractSubFuncNames(const std::string &p_sub_func_definition) {
   
    for(auto & i: ctre::range<expr>(p_sub_func_definition))
        if(auto m = ctre::match<expr>(i)){

            std::string str_def = m.get<2>().to_string();

            if(str_def.find("reverse") != std::string::npos)
                m_sub_reverse_name = m.get<1>().to_string();
            else if (str_def.find("splice") != std::string::npos)
                m_sub_splice_name = m.get<1>().to_string();
            else
                m_sub_swap_name = m.get<1>().to_string();
        }
}

void Decipher::ExtractDecipher(const std::string &p_decipher_func_definition) {

    auto m1 = ctre::match<R"(\.(..)\(.,(\d+)\))">(p_decipher_func_definition);

    std::stringstream ss(p_decipher_func_definition);

    std::string item;

    while (std::getline(ss, item, ';'))
        if(auto m2 = ctre::search<R"(\.(..)\(.,(\d+)\))">(item))
            m_decipher.emplace_back(m2.get<1>().to_string(), m1.get<2>().to_number());    
}

/*
 * Code below is copyright (c) 2018 Linus Kloeckner
*/

void Decipher::DecipherSignature(std::string *p_signature) {
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

void Decipher::LoadDecipher(const std::string &p_video_html) {
    std::string decipher_js = LoadDecipherJS(p_video_html);
    std::string decipher_func_name = LoadDecipherFuncName(decipher_js);
    std::string decipher_func_definition = LoadDecipherFuncDefinition(decipher_js, decipher_func_name);
    std::string sub_func_name = LoadSubFuncName(decipher_func_definition);
    std::string sub_func_definition = LoadSubFuncDefinition(decipher_js, sub_func_name);
    ExtractSubFuncNames(sub_func_definition);
    ExtractDecipher(decipher_func_definition);
}

void Decipher::SubReverse(std::string *p_a) {
    std::reverse(p_a->begin(), p_a->end());
}

void Decipher::SubSplice(std::string *p_a, int p_b) {
    p_a->erase(0, static_cast<uint64_t>(p_b));
}

void Decipher::SubSwap(std::string *p_a, int p_b) {
    char c = (*p_a)[0];
    (*p_a)[0] = (*p_a)[p_b % p_a->length()];
    (*p_a)[p_b] = c;
}
