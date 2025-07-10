# Enable strict compiler warnings and treat them as errors

if(MSVC)
    # Microsoft Visual C++ compiler flags
    add_compile_options(/W4 /WX /wd4100 /wd4201 /wd4505)
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _SCL_SECURE_NO_WARNINGS
    )
else()
    # Clang/GCC compiler flags
    add_compile_options(
        -Wall
        -Wextra
        -Werror
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wno-unused-parameter
        -Wno-missing-field-initializers
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(
            -Weverything
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-padded
        )
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(
            -Wduplicated-cond
            -Wlogical-op
            -Wnull-dereference
        )
    endif()
endif()

# Additional warning flags for debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        add_compile_options(-ftrapv -fstack-protector-all)
    endif()
endif()

# Enable link-time optimization for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_output)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "IPO/LTO not supported: ${ipo_output}")
    endif()
endif()