#ifndef OGGTUBE_PUBLIC_HEADER
#define OGGTUBE_PUBLIC_HEADER

/*
 * Oggtube API
 * Public Header
 */

/*
 * Acknowledgements:
 * Oggtube uses a combination of Microsoft's REST C++ SDK, Google's RE2 for generating precompiled regex
 * Kloeckner's JS decryption algorithms and FFmpeg's libavcodec for media transmuxing.
 */

namespace OggtubeAPI {


    /*
     * Checks that Oggtube initialised OK
     */
    bool startOggtube();

    /*
     * Downloads a YouTube URL as an .Ogg file
     */

    int download(const char string[12]);

    /*
     * Returns an error code
     * 0 - no error
     * 1 - indeterminable error
     * 2 - cipher error
     */

}




#endif // OGGTUBE_PUBLIC_HEADER