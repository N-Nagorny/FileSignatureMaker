# Boost

set(BOOST_VERSION_MIN "1.69.0")
set(BOOST_VERSION_CUR "1.77.0")

list(APPEND FIND_BOOST_COMPONENTS headers)
find_package(Boost ${BOOST_VERSION_MIN} REQUIRED COMPONENTS ${FIND_BOOST_COMPONENTS})

# cope with historical versions of FindBoost.cmake
if(DEFINED Boost_VERSION_STRING)
    set(Boost_VERSION_COMPONENTS "${Boost_VERSION_STRING}")
elseif(DEFINED Boost_VERSION_MAJOR)
    set(Boost_VERSION_COMPONENTS "${Boost_VERSION_MAJOR}.${Boost_VERSION_MINOR}.${Boost_VERSION_PATCH}")
elseif(DEFINED Boost_MAJOR_VERSION)
    set(Boost_VERSION_COMPONENTS "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
elseif(DEFINED Boost_VERSION)
    set(Boost_VERSION_COMPONENTS "${Boost_VERSION}")
else()
    message(FATAL_ERROR "Boost_VERSION_STRING is not defined")
endif()

if(Boost_VERSION_COMPONENTS VERSION_LESS BOOST_VERSION_MIN)
    message(FATAL_ERROR "Found Boost version " ${Boost_VERSION_COMPONENTS} " that is lower than the minimum version: " ${BOOST_VERSION_MIN})
elseif(Boost_VERSION_COMPONENTS VERSION_GREATER BOOST_VERSION_CUR)
    message(STATUS "Found Boost version " ${Boost_VERSION_COMPONENTS} " that is higher than the current tested version: " ${BOOST_VERSION_CUR})
else()
    message(STATUS "Found Boost version " ${Boost_VERSION_COMPONENTS})
endif()

if(DEFINED Boost_INCLUDE_DIRS)
    message(STATUS "Using Boost include directories at ${Boost_INCLUDE_DIRS}")
endif()

# this target is intended to just link it as a single Boost dependency
# and also provides a common location to inject some additional compile definitions
add_library(Boost INTERFACE)
target_link_libraries(Boost INTERFACE "${Boost_LIBRARIES}")


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

