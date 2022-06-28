include (CTest)

add_executable(OggtubeTest
        ../src/router.cpp
        ../src/utils.cpp
        ../src/decipher.cpp
        ../src/oggtube.cpp
        ../src/muxxer.cpp
        tests.cpp
        )

target_include_directories(OggtubeTest
        PRIVATE
        ../src/
        )

target_link_libraries(OggtubeTest
        OggtubeLib
        )

add_test(oggtube-ctest
        OggtubeTest)

find_program(VALGRIND "valgrind")

if(VALGRIND)
    add_custom_target(valgrind
        COMMAND "${VALGRIND}" --tool=memcheck --track-origins=yes --leak-check=full --show-reachable=no --num-callers=20 --track-fds=yes $<TARGET_FILE:Oggtube>)
endif()