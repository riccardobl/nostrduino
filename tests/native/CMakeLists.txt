cmake_minimum_required(VERSION 3.16)
project(nostrduino_native_tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
add_link_options(-fsanitize=address,undefined)

add_executable(test_encoding
    test_encoding.cpp
    ../../src/NostrEncoding.cpp
    ../../src/NostrString.cpp
)
target_include_directories(test_encoding PRIVATE ../../src)

add_executable(test_transport_helpers
    test_transport_helpers.cpp
    ../../src/NostrUrl.cpp
    ../../src/NostrString.cpp
)
target_include_directories(test_transport_helpers PRIVATE ../../src)
