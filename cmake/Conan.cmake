if(CONAN_EXPORTED)
    return()
endif()

if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.16.1/conan.cmake"
                  "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)

# checking the Conan version produces a more helpful message than the confusing errors
# that are reported when some dependency's recipe uses new features
set(CONAN_VERSION_MIN "1.33.0")
conan_check(VERSION ${CONAN_VERSION_MIN} REQUIRED)

conan_cmake_run(CONANFILE conanfile.txt
                BASIC_SETUP
                NO_OUTPUT_DIRS
                GENERATORS cmake_find_package
                KEEP_RPATHS)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
