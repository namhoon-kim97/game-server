#pragma once
#include "IOCPCore.h"
#include <vector>
#include <thread>
#include <atomic>

class WorkerThread
{
public:
    // threadCount 기본값: 논리 CPU 수 × 2
    void Start(IOCPCore* iocpCore, uint32 threadCount = 0);
    void Stop();
    void Join();

private:
    void Run();

    IOCPCore*               _iocpCore = nullptr;
    std::vector<std::thread> _threads;
    std::atomic<bool>        _running = false;
};
