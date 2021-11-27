#ifndef OGGTUBE_DEMUXER_H
#define OGGTUBE_DEMUXER_H

namespace Oggtube {
    class muxxer {

        int packet_pos;


    public:

        muxxer();

        void transmux(const char *in_filename, const char *out_filename);


    };

}


#endif // OGGTUBE_DEMUXER_H