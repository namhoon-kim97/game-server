#pragma once
#include "IOCPObject.h"
#include "Core/Types.h"

class IOCPCore
{
public:
    ~IOCPCore() { if (_hcp != INVALID_HANDLE_VALUE) CloseHandle(_hcp); }

    bool Init();

    // 소켓을 completion port에 등록 (completion key = IOCPObject*)
    bool Register(IOCPObject* obj);

    // GetQueuedCompletionStatus 한 번 호출 → IOCPObject::Dispatch() 위임
    // WorkerThread가 루프로 반복 호출
    bool Dispatch(uint32 timeoutMs = INFINITE);

    HANDLE GetHandle() const { return _hcp; }

private:
    HANDLE _hcp = INVALID_HANDLE_VALUE;
};
