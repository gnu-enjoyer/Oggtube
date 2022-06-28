cmake_minimum_required(VERSION 3.16)

project(Oggtube)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

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

FetchContent_Declare(
        fmt
        GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
        GIT_TAG b6f4ceaed0a0a24ccf575fab6c56dd50ccf6f1a9
)

FetchContent_MakeAvailable(zlib simdjson re2 fmt)

pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
        libavformat
        libavcodec
        libavutil
        )

add_executable(${PROJECT_NAME}
        src/router.cpp
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
        fmt
        )

target_link_libraries(${PROJECT_NAME}
        OggtubeLib)
        
enable_testing()

add_subdirectory(tests)