#include "muxxer.h"
#include "oggtube.h"
#include <iostream>

#include "ut.h"

/*
 * Acknowledgements:
 * Oggtube primarily uses Google's RE2 for generating precompiled regex,
 * Kloeckner's JS decryption algorithms, Boost μt for tests,
 * and FFmpeg's libavcodec for media transmuxing.
 */

int main() {

  using namespace boost::ut;

  /// examples:
  auto oggt = Oggtube();

  "Oggtube"_test = [&oggt] {
    if (auto result = oggt.parse("lhl9i9CI2-E")) {
      std::cout << *oggt.getBufferPtr();

      auto Mux = Muxxer(oggt.getBufferPtr()->c_str(), "test.ogg");

      expect(result) << "with Ogg muxxing";
    }
  };

  "No Muxxing"_test = [&oggt] {
    std::cout << "\n";

    if (auto result = oggt.parse("0FhFMkd4u_U")) {
      std::cout << *oggt.getBufferPtr();

      // Your muxxing solution here

      expect(result) << "without Ogg muxxing";
    }
  };

  return 0;
}
