file(GLOB_RECURSE SHARED_SOURCES
    "Core/*.cpp"
    "Protocol/*.cpp"
)

add_library(Shared STATIC ${SHARED_SOURCES})

# PUBLIC — Shared를 링크하는 Server, ClientStub도 이 경로를 자동으로 가져감
target_include_directories(Shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
