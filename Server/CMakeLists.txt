file(GLOB_RECURSE SERVER_SOURCES
    "src/*.cpp"
)

add_executable(Server ${SERVER_SOURCES})

target_include_directories(Server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(Server PRIVATE
    Shared
    Ws2_32      # WinSock2
    Mswsock     # AcceptEx 등 확장 소켓 함수
)

# SQLite threadsafe 모드 (2 = serialized)
target_compile_definitions(Server PRIVATE SQLITE_THREADSAFE=2)
