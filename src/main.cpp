#include <iostream>
#include "oggtube.h"
#include "muxxer.h"

/*
 * Acknowledgements:
 * Oggtube primarily uses Google's RE2 for generating precompiled regex,
 * Kloeckner's JS decryption algorithms,
 * and FFmpeg's libavcodec for media transmuxing.
 */

int main() {

    ///examples:
    auto oggt = Oggtube();

    ///with Ogg muxxing:
    auto muxxy = Muxxer();

    if(oggt.parse("lhl9i9CI2-E")){
        std::cout << *oggt.getBufferPtr();
        muxxy.transmux(oggt.getBufferPtr()->c_str(), "test.ogg");
    }

    ///without Ogg muxxing:
    std::cout << "\n";

    if(oggt.parse("0FhFMkd4u_U")) {
        std::cout << *oggt.getBufferPtr();
        //Your muxxing solution here
    }

    return 0;
}
