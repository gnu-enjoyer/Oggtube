cmake_minimum_required(VERSION 3.16)

project(Oggtube)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${PROJECT_SOURCE_DIR}/includes)

add_library(${PROJECT_NAME}
        sources/oggtube.cpp
        sources/demux.cpp
        sources/rest.cpp
        )

add_executable(OggtubeCLI
        example/main.cpp
        )

if (WIN32)

    #vcpkg
    find_package(FFMPEG REQUIRED)
    find_package(re2 CONFIG REQUIRED)
    find_package(cpprestsdk REQUIRED)
    find_package(OpenSSL REQUIRED)

    target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES})

    target_link_libraries(${PROJECT_NAME} PRIVATE
            re2::re2
            cpprestsdk::cpprest
            OpenSSL::SSL OpenSSL::Crypto
            )

else()

    #pkgconfig
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
            libavformat
            libavcodec
            libavutil
            )

        target_link_libraries(${PROJECT_NAME}
            PkgConfig::LIBAV)

    pkg_check_modules(RE2 re2)
    if(RE2_FOUND)
        add_library(re2::re2 INTERFACE IMPORTED)
        set_target_properties(re2::re2 PROPERTIES
                INTERFACE_COMPILE_OPTIONS "${RE2_CFLAGS}"
                INTERFACE_LINK_LIBRARIES "${RE2_LDFLAGS}"
                )
        target_link_libraries(${PROJECT_NAME} re2::re2)
    endif()

    find_package(cpprestsdk REQUIRED)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(${PROJECT_NAME}
            cpprestsdk::cpprest
            OpenSSL::SSL OpenSSL::Crypto)

endif()

target_link_libraries(OggtubeCLI PRIVATE ${PROJECT_NAME})