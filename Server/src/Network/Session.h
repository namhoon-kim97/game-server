#pragma once
#include "Core/IOCPObject.h"
#include "Core/Types.h"
#include "Network/RecvBuffer.h"
#include "Network/SendBuffer.h"
#include <WinSock2.h>
#include <memory>
#include <atomic>

// I/O pending 중 Session 해제 방지 — shared_ptr로 ref count 유지
struct RecvEvent : public IOCPEvent
{
    RecvEvent() : IOCPEvent(IOCPEventType::RECV) {}
    std::shared_ptr<class Session> owner;
};

struct SendEvent : public IOCPEvent
{
    SendEvent() : IOCPEvent(IOCPEventType::SEND) {}
    std::shared_ptr<class Session> owner;
};

class Session : public IOCPObject, public std::enable_shared_from_this<Session>
{
public:
    Session();
    ~Session();

    // Listener::HandleAccept에서 accept 후 호출
    void Init(SOCKET socket, uint32 sessionIdx);
    void Close();
    void Send(const BYTE* data, int32 len);

    // IOCPCore::Register 후 Listener에서 직접 호출
    void IssueRecv();

    void Dispatch(IOCPEvent* event, DWORD bytesTransferred) override;

    SOCKET GetSocket()     const { return _socket; }
    uint32 GetSessionIdx() const { return _sessionIdx; }
    uint64 GetSessionId()  const { return _sessionId; }
    bool   IsConnected()   const { return _connected.load(); }

protected:
    virtual void OnConnected()    {}
    virtual void OnDisconnected() {}

    // ProcessPackets에서 완전한 패킷(헤더 포함) 수신 시 호출
    // Day 3에서 PacketDispatcher::Dispatch로 교체
    virtual void OnRecv(BYTE* buffer, int32 len) {}

private:
    void IssueSend();
    void ProcessPackets();

    static std::atomic<uint64> s_nextSessionId;

    SOCKET    _socket     = INVALID_SOCKET;
    uint32    _sessionIdx = 0;
    uint64    _sessionId  = 0;

    RecvBuffer _recvBuffer;
    SendBuffer _sendBuffer;

    RecvEvent  _recvEvent;
    SendEvent  _sendEvent;

    std::atomic<bool> _connected{ false };
};
