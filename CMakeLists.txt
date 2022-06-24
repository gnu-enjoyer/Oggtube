cmake_minimum_required(VERSION 3.16)

project(Oggtube)

include(FetchContent)

include (CTest)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

find_package(PkgConfig REQUIRED)

find_package(OpenSSL REQUIRED)

FetchContent_Declare(
        zlib
        GIT_REPOSITORY "https://github.com/madler/zlib"
        GIT_TAG "cacf7f1d4e3d44d871b605da3b647f07d718623f"
)

FetchContent_Declare(
        simdjson
        GIT_REPOSITORY "https://github.com/simdjson/simdjson"
        GIT_TAG "17f3148ac73da9895bdbd26ed2c4848fd77571a8"
)

FetchContent_Declare(
        re2
        GIT_REPOSITORY "https://github.com/google/re2"
        GIT_TAG "0dade9ff39bb6276f18dd6d4bc12d3c20479ee24"
)

FetchContent_MakeAvailable(zlib simdjson re2)

pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
        libavformat
        libavcodec
        libavutil
        )

add_executable(${PROJECT_NAME}
        src/utils.cpp
        src/decipher.cpp
        src/oggtube.cpp
        src/muxxer.cpp
        src/main.cpp
        )

add_library(OggtubeLib
        INTERFACE)

target_link_libraries(OggtubeLib
        INTERFACE
        PkgConfig::LIBAV
        OpenSSL::SSL OpenSSL::Crypto
        libz.a
        simdjson
        re2
        )

target_link_libraries(${PROJECT_NAME}
        OggtubeLib)
        
enable_testing()

add_test(oggtube_test
        Oggtube
        )

find_program(VALGRIND "valgrind")

if(VALGRIND)
    add_custom_target(valgrind
        COMMAND "${VALGRIND}" --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes $<TARGET_FILE:Oggtube>)
endif()