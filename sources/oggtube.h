#ifndef OGGTUBE_PRIVATE_HEADER
#define OGGTUBE_PRIVATE_HEADER

/*
 * Acknowledgements:
 * Oggtube uses a combination of Microsoft's REST C++ SDK, Google's RE2 for generating precompiled regex
 * Kloeckner's JS decryption algorithms and FFmpeg's libavcodec for media transmuxing.
 */

#include <vector>
#include <string>

namespace Oggtube {

    class regexes {
    public:

        std::vector<void*> re_list;
        regexes(std::initializer_list<void*> rgx) : re_list(rgx) {}

    };
    int parse_input(std::string &in);

    void replaceAll(std::string &source, const std::string_view &from, const std::string_view &to);
    void HTMLtoUTF8(std::string &scraped);
}

namespace kloeckner {
    class Decipher {
    public:
        static Decipher &Instance(const std::string &p_video_html) {
            static Decipher _instance(p_video_html);
            return _instance;
       }

        void DecipherSignature(std::string *p_signature);

    private:
        Decipher() = default;

        explicit Decipher(const std::string &p_video_html) {
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
    };
}





#endif // OGGTUBE_PRIVATE_HEADER