cmake_minimum_required(VERSION 3.5)

project(FileSignatureMaker LANGUAGES CXX)

set(USE_CONAN ON CACHE BOOL "Use Conan to acquire dependencies")
mark_as_advanced(FORCE USE_CONAN)

if(USE_CONAN)
    include(cmake/Conan.cmake)
endif()

include(cmake/Dependencies.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

set(SOURCES
    src/main.cpp
    src/per_block_file_processor.cpp
    src/thread_pool.cpp
)

set(HEADERS
    src/
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${HEADERS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost
    OpenSSL
    $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Linux>:Threads>
)
