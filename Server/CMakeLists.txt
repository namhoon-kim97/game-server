file(GLOB_RECURSE SERVER_SOURCES
    "src/*.cpp"
)

# sqlite3 (C 코드라 별도 타겟)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/sqlite/sqlite3.c)
    add_library(sqlite3 STATIC vendor/sqlite/sqlite3.c)
    target_include_directories(sqlite3 PUBLIC vendor/sqlite)
    target_compile_definitions(sqlite3 PUBLIC SQLITE_THREADSAFE=2)
    set(SQLITE_TARGET sqlite3)
endif()

add_executable(Server ${SERVER_SOURCES})

target_include_directories(Server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
)

# SQLite threadsafe 모드 (2 = serialized)
target_compile_definitions(Server PRIVATE SQLITE_THREADSAFE=2)

target_link_libraries(Server PRIVATE
    Shared
    Ws2_32      # WinSock2
    Mswsock     # AcceptEx 등 확장 소켓 함수
    $<$<BOOL:${SQLITE_TARGET}>:${SQLITE_TARGET}>
)
