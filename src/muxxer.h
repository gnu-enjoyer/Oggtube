#ifndef OGGTUBE_MUXXER_H
#define OGGTUBE_MUXXER_H


class Muxxer {

    class AVFormatContext* input_format_context = nullptr;    
    class AVFormatContext* output_format_context = nullptr;

    int* streams_list = nullptr;
    int stream_index = 0;
    int number_of_streams = 0;

public:
    Muxxer(const char *in_filename, const char *out_filename);
    ~Muxxer();
    
};


#endif //OGGTUBE_MUXXER_H
