# Helper macros to find all project dependencies

# Sets default find_package behavior
cmake_policy(SET CMP0074 NEW)

# Find essential system libraries
find_package(Threads REQUIRED)

# Find OpenSSL for networking
find_package(OpenSSL REQUIRED)

# Find optional dependencies with fallbacks
macro(find_optional_dependency)
    set(OPTIONAL_VALUE "QUIET")
    if(${ARGC} GREATER 1 AND ARGV1 STREQUAL "REQUIRED")
        set(OPTIONAL_VALUE "REQUIRED")
    endif()

    find_package(${ARGV0} ${OPTIONAL_VALUE})
    if(${${ARGV0}_FOUND} OR ${ARGV0}_FOUND)
        message(STATUS "Found ${ARGV0}: ${${ARGV0}_VERSION}")
        add_compile_definitions(HAS_${ARGV0}=1)
    else()
        message(STATUS "Optional dependency ${ARGV0} not found")
    endif()
endmacro()

# Check for specific dependencies
find_optional_dependency(ZLIB)
find_optional_dependency(Protobuf)
find_optional_dependency(GoogleTest 1.10.0)

# Custom find module for Jemalloc (memory allocator)
find_path(JEMALLOC_INCLUDE_DIR jemalloc/jemalloc.h)
find_library(JEMALLOC_LIBRARY NAMES jemalloc)

if(JEMALLOC_INCLUDE_DIR AND JEMALLOC_LIBRARY)
    set(JEMALLOC_FOUND TRUE)
    add_compile_definitions(USE_JEMALLOC=1)
    message(STATUS "Found jemalloc: ${JEMALLOC_LIBRARY}")
else()
    message(STATUS "jemalloc not found, using system allocator")
endif()

# Handle found dependencies
set(PROJECT_DEPENDENCIES
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(ZLIB_FOUND)
    list(APPEND PROJECT_DEPENDENCIES ZLIB::ZLIB)
endif()

if(JEMALLOC_FOUND)
    list(APPEND PROJECT_DEPENDENCIES ${JEMALLOC_LIBRARY})
endif()