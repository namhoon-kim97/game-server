file(GLOB_RECURSE CLIENT_SOURCES
    "src/*.cpp"
)

add_executable(ClientStub ${CLIENT_SOURCES})

target_include_directories(ClientStub PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(ClientStub PRIVATE
    Shared
    Ws2_32
    Mswsock
)
