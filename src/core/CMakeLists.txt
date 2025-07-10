# src/core/CMakeLists.txt
add_library(blitzdb_core STATIC
    storage/in_memory.cpp
    storage/persistent.cpp
    # Add other core source files
)

target_include_directories(blitzdb_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(blitzdb_core PRIVATE
    asio::asio  # If core needs ASIO
)