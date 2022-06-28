#include "router.h"

/*
 * Acknowledgements:
 * Oggtube primarily uses Google's RE2 for generating precompiled regex,
 * Kloeckner's JS decryption algorithms, Boost μt for tests,
 * and FFmpeg's libavcodec for media transmuxing.
 */

int main() {

  // probably put this behind nginx 
  Router Router;
  Router.listen(8080);

  return 0;
}
