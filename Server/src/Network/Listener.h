#pragma once
#include "Core/IOCPObject.h"
#include "Core/IOCPCore.h"
#include "Core/Types.h"
#include <WinSock2.h>
#include <MSWSock.h>
#include <vector>
#include <memory>

// AcceptEx용 이벤트 - accept 완료 시 Dispatch로 전달
struct AcceptEvent : public IOCPEvent
{
    AcceptEvent() : IOCPEvent(IOCPEventType::ACCEPT) {}

    SOCKET clientSocket = INVALID_SOCKET;

    // AcceptEx가 로컬/원격 주소를 이 버퍼에 기록
    // ((sizeof(SOCKADDR_IN) + 16) * 2) 크기 필요
    BYTE addrBuf[88] = {};
};

class Listener : public IOCPObject
{
public:
    ~Listener();

    bool Start(IOCPCore* iocpCore, uint16 port, int32 backlog = SOMAXCONN);
    void Dispatch(IOCPEvent* event, DWORD bytesTransferred) override;

private:
    bool RegisterAccept(AcceptEvent* event);
    void HandleAccept(AcceptEvent* event);

    IOCPCore*  _iocpCore   = nullptr;
    SOCKET     _listenSock = INVALID_SOCKET;
    LPFN_ACCEPTEX _fnAcceptEx = nullptr;

    // AcceptEvent 소유 - 종료 시 정리
    std::vector<std::unique_ptr<AcceptEvent>> _acceptEvents;
};
