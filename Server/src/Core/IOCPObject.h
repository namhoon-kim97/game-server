#pragma once
#include <WinSock2.h>
#include <Windows.h>

enum class IOCPEventType
{
    ACCEPT,
    RECV,
    SEND,
};

// OVERLAPPED를 상속 - GetQueuedCompletionStatus가 반환하는 포인터를 캐스팅해서 타입 정보 추출
struct IOCPEvent : public OVERLAPPED
{
    IOCPEvent(IOCPEventType type) : type(type)
    {
        // OVERLAPPED 멤버 0 초기화
        memset(static_cast<OVERLAPPED*>(this), 0, sizeof(OVERLAPPED));
    }

    IOCPEventType type;
};

// IOCP에 등록 가능한 객체의 기반 클래스 (Listener, Session이 상속)
class IOCPObject
{
public:
    virtual ~IOCPObject() = default;

    // I/O 완료 통지 처리 - 상속 클래스에서 구현
    virtual void Dispatch(IOCPEvent* event, DWORD bytesTransferred) = 0;

    HANDLE GetHandle() const { return _handle; }

protected:
    HANDLE _handle = INVALID_HANDLE_VALUE;
};
