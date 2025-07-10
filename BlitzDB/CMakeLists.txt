# blitzdb/CMakeLists.txt - Main executable configuration

# Define the executable
add_executable(blitzdb 
    blitzdb.cpp
    blitzdb.h
)

# Modern CMake standards
set_target_properties(blitzdb PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON  # Recommended for modern C++
)

# --------------------------------------------------
# ASIO Configuration (Updated)
# --------------------------------------------------
# Option 1: Use if ASIO was installed system-wide or via vcpkg
# Try modern CMake config first

# Direct ASIO configuration (bypass find_package)
set(ASIO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/asio/asio-1.34.2/asio/include")

# Create interface library for ASIO
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${ASIO_INCLUDE_DIR})
target_compile_definitions(asio INTERFACE ASIO_STANDALONE=1)

# Create alias for consistent naming
add_library(asio::asio ALIAS asio)

# --------------------------------------------------
# Dependencies
# --------------------------------------------------

add_subdirectory(
    ${CMAKE_SOURCE_DIR}/src/network
    ${CMAKE_CURRENT_BINARY_DIR}/network
)

add_subdirectory(
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_BINARY_DIR}/core
)

target_link_libraries(blitzdb PRIVATE
    blitzdb_core
    blitzdb_network
    asio::asio
)

# --------------------------------------------------
# Installation & Packaging
# --------------------------------------------------
install(TARGETS blitzdb
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT Runtime
)

# --------------------------------------------------
# Platform-Specific Configurations
# --------------------------------------------------
if(WIN32)
    target_compile_definitions(blitzdb PRIVATE
        _WIN32_WINNT=0x0A00  # Windows 10+
        WIN32_LEAN_AND_MEAN
        NOMINMAX  # Prevent min/max macros
    )
    target_compile_options(blitzdb PRIVATE /W4 /WX)
else()
    target_compile_options(blitzdb PRIVATE
        -Wall
        -Wextra
        -Werror
        -pedantic
    )
endif()

# --------------------------------------------------
# Build Configurations
# --------------------------------------------------
# Debug symbols for all builds
target_compile_options(blitzdb PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/Zi>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g>
)

# LTO for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported)
    if(ipo_supported)
        set_property(TARGET blitzdb PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

# --------------------------------------------------
# Testing
# --------------------------------------------------
#if(BUILD_TESTING)
#    enable_testing()
#    add_subdirectory(tests)
#endif()

# --------------------------------------------------
# Optional Features
# --------------------------------------------------
option(WITH_SSL "Enable SSL support" OFF)
if(WITH_SSL)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(blitzdb PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(blitzdb PRIVATE USE_SSL=1)
endif()
