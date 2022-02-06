# OpenSSL

find_package(OpenSSL REQUIRED)
if(DEFINED OPENSSL_INCLUDE_DIR)
    message(STATUS "Using OpenSSL include directory at ${OPENSSL_INCLUDE_DIR}")
endif()

# this target is intended to just link it as a single OpenSSL dependency
# and also provides a common location to inject some additional compile definitions
add_library(OpenSSL INTERFACE)
target_link_libraries(OpenSSL INTERFACE OpenSSL::Crypto)


# pthread

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_package(Threads)

    # this target is intended to just link it as a single Threads dependency
    # and also provides a common location to inject some additional compile definitions
    add_library(Threads INTERFACE)
    target_link_libraries(Threads INTERFACE Threads::Threads)
endif()
