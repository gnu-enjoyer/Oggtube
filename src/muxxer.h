#ifndef OGGTUBE_MUXXER_H
#define OGGTUBE_MUXXER_H


class Muxxer {

public:

    bool transmux(const char *in_filename, const char *out_filename);

    Muxxer() = default;
    ~Muxxer() = default;

};


#endif //OGGTUBE_MUXXER_H
