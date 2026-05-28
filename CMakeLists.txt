cmake_minimum_required(VERSION 3.20)
project(GameServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows 전처리기 정의
add_compile_definitions(
    _WIN32_WINNT=0x0601
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
    _WINSOCK_DEPRECATED_NO_WARNINGS
)

# /W4 /WX — 경고를 오류로, /utf-8 — 소스 인코딩
if(MSVC)
    add_compile_options(/W4 /WX /utf-8)
endif()

add_subdirectory(Shared)
add_subdirectory(Server)
add_subdirectory(ClientStub)
