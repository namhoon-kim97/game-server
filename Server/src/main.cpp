#include "Core/IOCPCore.h"
#include "Core/WorkerThread.h"
#include "Core/Logger.h"
#include "Core/Types.h"
#include "Network/Listener.h"
#include "Network/Session.h"
#include <WinSock2.h>
#include <cstdio>
#include <csignal>
#include <thread>
#include <chrono>

static volatile bool g_running = true;

void SignalHandler(int)
{
    g_running = false;
}

int main()
{
    std::signal(SIGINT,  SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    // WinSock 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("[ERROR] WSAStartup failed\n");
        return 1;
    }

    // IOCP completion port 생성
    IOCPCore iocpCore;
    if (!iocpCore.Init())
    {
        LOG_ERR("IOCPCore::Init failed");
        WSACleanup();
        return 1;
    }

    // 리스너 시작 (포트 7777)
    constexpr uint16 PORT = 7777;
    Listener listener;
    listener.SetSessionHandler([](std::shared_ptr<Session> session)
    {
        LOG_INFO("New session connected (idx=%u, id=%llu)",
                 session->GetSessionIdx(), session->GetSessionId());
    });

    // Start() 내부에서 IOCP 등록까지 처리함
    if (!listener.Start(&iocpCore, PORT))
    {
        LOG_ERR("Listener::Start failed on port %d", PORT);
        WSACleanup();
        return 1;
    }

    // 워커 스레드 시작 (기본: 논리 CPU × 2)
    WorkerThread workerThread;
    workerThread.Start(&iocpCore);

    LOG_INFO("Server started on port %d", PORT);
    printf("Server started on port %d — press Ctrl+C to stop\n", PORT);

    while (g_running)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    LOG_INFO("Server shutting down...");
    workerThread.Stop();
    workerThread.Join();

    WSACleanup();
    return 0;
}