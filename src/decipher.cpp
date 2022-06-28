#include "utils.h"
#include "decipher.h"
#include "re2/re2.h"
#include <regex>


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

    for (auto &r : re_list){
        delete(static_cast<RE2*>(r));
    }

}

std::string Decipher::LoadDecipherFuncName(const std::string &p_decipher_js) {
    int x = 0;
    re2::StringPiece y;
    re2::StringPiece z;


//    for (auto &e: rgx.re_list) {
    for (auto &e: rPtr->re_list) {
        if (static_cast<RE2*>(e)->ok()) {
            auto found = static_cast<RE2*>(e)->Match(p_decipher_js, 0, p_decipher_js.size(), RE2::UNANCHORED, &y, x);

            if (found) {
                RE2::PartialMatch(p_decipher_js, *static_cast<RE2*>(e), &z);
                return z.ToString();
            }
        }
    }

    Log.write("[Decrypt] Could not find decipher function name!", true);
    throw std::runtime_error("Could not find decipher function name!");

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

std::string Decipher::LoadDecipherJS(const std::string &p_video_html) {
    std::regex expr_player_url(R"((?:PLAYER_JS_URL|jsUrl)\"\s*:\s*\"([^"]+))");
    std::smatch matches_player_url;
    std::regex_search(p_video_html, matches_player_url, expr_player_url);

    if (matches_player_url.empty()) {
        throw std::runtime_error("Could not find player URL!");
    }

    std::string str_decipher = matches_player_url[1];
    std::string str_body;
    Utils::yt_to_string(str_decipher.c_str(), str_body);

    return str_body;
}


std::string Decipher::LoadDecipherFuncDefinition(const std::string &p_decipher_js,
                                                            const std::string &p_decipher_func_name) {

    //TBD: Move to RE2
    std::regex expr_sub_func_definition(p_decipher_func_name + R"(=function\(.+?\)\{(.+?)\})");
    std::smatch matches_sub_funcs;

    std::regex_search(p_decipher_js, matches_sub_funcs, expr_sub_func_definition);
    return matches_sub_funcs[1];
}

std::string Decipher::LoadSubFuncName(const std::string &p_decipher_func_definition) {

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

std::string Decipher::LoadSubFuncDefinition(const std::string &p_decipher_js,
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

void Decipher::ExtractSubFuncNames(const std::string &p_sub_func_definition) {

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

void Decipher::ExtractDecipher(const std::string &p_decipher_func_definition) {

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

