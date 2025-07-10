# src/network/CMakeLists.txt
add_library(blitzdb_network STATIC
    server.cpp
    connection.cpp  # Only .cpp files should be listed here
)

# Headers should be listed separately for IDE organization
set(NETWORK_HEADERS
    server.h
    connection.h
)

# Modern CMake: Mark headers for proper IDE integration
target_sources(blitzdb_network PUBLIC
    ${NETWORK_HEADERS}
)

# Link dependencies
target_link_libraries(blitzdb_network PRIVATE
    blitzdb_core
    asio::asio
)

# Set output directories (modern approach)
set_target_properties(blitzdb_network PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    POSITION_INDEPENDENT_CODE ON  # Recommended for static libraries
)

# Install rules (optional for production)
install(TARGETS blitzdb_network
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT Runtime
)

# Include directories (if needed)
target_include_directories(blitzdb_network PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)