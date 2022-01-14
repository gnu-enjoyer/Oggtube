#ifndef OGGTUBE_DECIPHER_H
#define OGGTUBE_DECIPHER_H

#include <vector>
#include <string>

class Regexes {

public:

    ~Regexes();
    std::vector<void*> re_list;
    Regexes(std::initializer_list<void*> rgx) : re_list(rgx) {}

};

class Decipher {
    Decipher() = default;

    static Regexes* AllocateRegex();
    Regexes* rPtr = nullptr;

    explicit Decipher(const std::string &p_video_html) {
            rPtr = AllocateRegex();
            LoadDecipher(p_video_html);
        }

        void LoadDecipher(const std::string &p_video_html);


        std::string LoadDecipherJS(const std::string &p_video_html);

        std::string LoadDecipherFuncName(const std::string &p_decipher_js);

        std::string
        LoadDecipherFuncDefinition(const std::string &p_decipher_js, const std::string &p_decipher_func_name);

        std::string LoadSubFuncName(const std::string &p_decipher_func_definition);

        std::string LoadSubFuncDefinition(const std::string &p_decipher_js, const std::string &p_sub_func_name);

        void ExtractSubFuncNames(const std::string &p_sub_func_definition);

        void ExtractDecipher(const std::string &p_decipher_func_definition);

        void SubReverse(std::string *p_a);

        void SubSplice(std::string *p_a, int p_b);

        void SubSwap(std::string *p_a, int p_b);

        std::string m_sub_reverse_name;
        std::string m_sub_splice_name;
        std::string m_sub_swap_name;

        std::vector<std::tuple<std::string, int>> m_decipher;

public:

    static Decipher &Instance(const std::string &p_video_html) {
        static Decipher _instance(p_video_html);
        return _instance;
    }

    void DecipherSignature(std::string *p_signature);

    };


#endif //OGGTUBE_DECIPHER_H
