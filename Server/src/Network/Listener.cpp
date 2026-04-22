#include "Network/Listener.h"
#include "Network/Session.h"
#include <WS2tcpip.h>

Listener::~Listener()
{
    if (_listenSock != INVALID_SOCKET)
        closesocket(_listenSock);

    for (auto& e : _acceptEvents)
    {
        if (e->clientSocket != INVALID_SOCKET)
            closesocket(e->clientSocket);
    }
}

bool Listener::Start(IOCPCore* iocpCore, uint16 port, int32 backlog)
{
    _iocpCore = iocpCore;

    _listenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (_listenSock == INVALID_SOCKET)
        return false;

    // AcceptEx 함수 포인터 로드 - 구현이 Mswsock.dll에 있어서 런타임 로드 필요
    GUID guidAcceptEx = WSAID_ACCEPTEX;
    DWORD bytes = 0;
    if (WSAIoctl(_listenSock, SIO_GET_EXTENSION_FUNCTION_POINTER,
                 &guidAcceptEx, sizeof(guidAcceptEx),
                 &_fnAcceptEx, sizeof(_fnAcceptEx),
                 &bytes, nullptr, nullptr) == SOCKET_ERROR)
        return false;

    _handle = reinterpret_cast<HANDLE>(_listenSock);
    if (!_iocpCore->Register(this))
        return false;

    SOCKADDR_IN addr = {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(_listenSock, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        return false;

    if (listen(_listenSock, backlog) == SOCKET_ERROR)
        return false;

    for (int32 i = 0; i < 10; i++)
    {
        auto event = std::make_unique<AcceptEvent>();
        if (RegisterAccept(event.get()))
            _acceptEvents.push_back(std::move(event));
    }

    return !_acceptEvents.empty();
}

bool Listener::RegisterAccept(AcceptEvent* event)
{
    event->clientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
                                    nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (event->clientSocket == INVALID_SOCKET)
        return false;

    memset(static_cast<OVERLAPPED*>(event), 0, sizeof(OVERLAPPED));

    DWORD bytes = 0;
    if (_fnAcceptEx(_listenSock, event->clientSocket,
                    event->addrBuf, 0,
                    sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
                    &bytes, event) == FALSE)
    {
        // 비동기 등록 성공 시 ERROR_IO_PENDING - 이건 정상
        if (WSAGetLastError() != ERROR_IO_PENDING)
        {
            closesocket(event->clientSocket);
            event->clientSocket = INVALID_SOCKET;
            return false;
        }
    }

    return true;
}

void Listener::Dispatch(IOCPEvent* event, DWORD bytesTransferred)
{
    AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(event);
    HandleAccept(acceptEvent);
    RegisterAccept(acceptEvent); // 다음 연결 대기 재등록
}

void Listener::HandleAccept(AcceptEvent* event)
{
    if (setsockopt(event->clientSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                   reinterpret_cast<char*>(&_listenSock), sizeof(_listenSock)) == SOCKET_ERROR)
    {
        closesocket(event->clientSocket);
        event->clientSocket = INVALID_SOCKET;
        return;
    }

    // TODO: Session 생성 및 IOCPCore 등록 (Session 구현 후 연결)
}
