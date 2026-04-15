#include "WorkerThread.h"

void WorkerThread::Start(IOCPCore* iocpCore, uint32 threadCount)
{
    _iocpCore = iocpCore;
    _running  = true;

    if (threadCount == 0)
        threadCount = std::thread::hardware_concurrency() * 2;

    for (uint32 i = 0; i < threadCount; i++)
        _threads.emplace_back(&WorkerThread::Run, this);
}

void WorkerThread::Stop()
{
    _running = false;

    // GQCS에서 블로킹 중인 스레드를 깨우기 위해 더미 완료 통지 전송
    for (size_t i = 0; i < _threads.size(); i++)
        PostQueuedCompletionStatus(_iocpCore->GetHandle(), 0, 0, nullptr);
}

void WorkerThread::Join()
{
    for (auto& t : _threads)
        if (t.joinable()) t.join();
}

void WorkerThread::Run()
{
    while (_running)
        _iocpCore->Dispatch();
}
