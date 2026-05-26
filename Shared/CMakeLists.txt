file(GLOB_RECURSE SHARED_SOURCES
    "Core/*.cpp"
    "Protocol/*.cpp"
)

if(SHARED_SOURCES)
    add_library(Shared STATIC ${SHARED_SOURCES})
    # PUBLIC — Shared를 링크하는 Server, ClientStub도 이 경로를 자동으로 가져감
    target_include_directories(Shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
else()
    # .cpp 파일 추가 전 임시 INTERFACE (Logger.cpp 등 추가되면 위 분기로 이동)
    add_library(Shared INTERFACE)
    target_include_directories(Shared INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
