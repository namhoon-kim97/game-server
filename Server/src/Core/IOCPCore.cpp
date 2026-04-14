#include "IOCPCore.h"

bool IOCPCore::Init()
{
    // 두 번째 인자 0 = 기존 핸들과 연결 없이 새 completion port 생성
    // 네 번째 인자 0 = 동시 실행 스레드 수를 논리 CPU 수에 맡김
    _hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    return _hcp != INVALID_HANDLE_VALUE;
}

bool IOCPCore::Register(IOCPObject* obj)
{
    // completion key로 IOCPObject* 전달 → Dispatch 시 캐스팅해서 사용
    return CreateIoCompletionPort(obj->GetHandle(), _hcp,
                                  reinterpret_cast<ULONG_PTR>(obj), 0) != nullptr;
}

bool IOCPCore::Dispatch(uint32_t timeoutMs)
{
    DWORD       bytes     = 0;
    ULONG_PTR   key       = 0;
    OVERLAPPED* overlapped = nullptr;

    BOOL ret = GetQueuedCompletionStatus(_hcp, &bytes, &key, &overlapped, timeoutMs);

    if (overlapped == nullptr)
        return false; // 타임아웃 또는 오류

    IOCPObject* obj   = reinterpret_cast<IOCPObject*>(key);
    IOCPEvent*  event = static_cast<IOCPEvent*>(overlapped);

    obj->Dispatch(event, bytes);
    return true;
}
