#include "Network/Session.h"

#include <MSWSock.h>

#include "Core/Logger.h"
#include "Protocol/PacketHeader.h"

std::atomic<uint64> Session::s_nextSessionId{1};

Session::Session() : _recvBuffer(MIN_RECV_SIZE), _sendBuffer(SOCKET_BUF_SIZE) {}

Session::~Session() {
  if (_socket != INVALID_SOCKET) closesocket(_socket);
}

void Session::Init(SOCKET socket, uint32 sessionIdx) {
  _socket = socket;
  _sessionIdx = sessionIdx;
  _sessionId = s_nextSessionId.fetch_add(1);
  _handle = reinterpret_cast<HANDLE>(_socket);
  _connected = true;
  OnConnected();
}

void Session::Close() {
  // exchange로 중복 호출 방지
  if (!_connected.exchange(false)) return;

  closesocket(_socket);
  _socket = INVALID_SOCKET;
  OnDisconnected();
}

void Session::Send(const BYTE* data, int32 len) {
  if (!_connected.load()) return;

  // Push가 true를 반환하면 WSASend를 처음 등록해야 함
  // false면 이미 pending 중 — 버퍼에만 쌓임, OnSend 완료 시 자동 전송
  if (_sendBuffer.Push(data, len)) IssueSend();
}

void Session::IssueRecv() {
  if (!_connected.load()) return;

  // WSARecv 완료 통보까지 Session이 해제되지 않도록 ref count 증가
  _recvEvent.owner = shared_from_this();

  WSABUF wsaBuf;
  wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.GetWriteBuffer());
  wsaBuf.len = static_cast<ULONG>(_recvBuffer.GetFreeSize());

  DWORD flags = 0;
  DWORD recvBytes = 0;

  // OVERLAPPED 재사용 전 초기화 (type/owner 필드는 건드리지 않음)
  memset(static_cast<OVERLAPPED*>(&_recvEvent), 0, sizeof(OVERLAPPED));

  int result =
      WSARecv(_socket, &wsaBuf, 1, &recvBytes, &flags, &_recvEvent, nullptr);

  if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
    LOG_ERR("IssueRecv WSARecv failed: %d (sessionIdx=%d)", WSAGetLastError(),
            _sessionIdx);
    _recvEvent.owner = nullptr;
    Close();
  }
}

void Session::IssueSend() {
  if (!_connected.load()) return;

  _sendEvent.owner = shared_from_this();

  WSABUF wsaBuf;
  wsaBuf.buf = reinterpret_cast<char*>(_sendBuffer.GetSendBuffer());
  wsaBuf.len = static_cast<ULONG>(_sendBuffer.GetDataSize());

  if (wsaBuf.len == 0) {
    _sendEvent.owner = nullptr;
    return;
  }

  DWORD flags = 0;
  DWORD sendBytes = 0;

  memset(static_cast<OVERLAPPED*>(&_sendEvent), 0, sizeof(OVERLAPPED));

  int result =
      WSASend(_socket, &wsaBuf, 1, &sendBytes, flags, &_sendEvent, nullptr);

  if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
    LOG_ERR("IssueSend WSASend failed: %d (sessionIdx=%d)", WSAGetLastError(),
            _sessionIdx);
    _sendEvent.owner = nullptr;
    Close();
  }
}

void Session::Dispatch(IOCPEvent* event, DWORD bytesTransferred) {
  switch (event->type) {
    case IOCPEventType::RECV: {
      RecvEvent* recvEvent = static_cast<RecvEvent*>(event);
      recvEvent->owner = nullptr;  // ref count 해제

      if (bytesTransferred == 0) {
        // TCP FIN — 클라이언트 정상 종료
        Close();
        return;
      }

      _recvBuffer.OnWrite(static_cast<int32>(bytesTransferred));
      ProcessPackets();
      _recvBuffer.Clean();
      IssueRecv();  // 다음 recv 재등록
    } break;

    case IOCPEventType::SEND: {
      SendEvent* sendEvent = static_cast<SendEvent*>(event);
      sendEvent->owner = nullptr;

      if (bytesTransferred == 0) {
        Close();
        return;
      }

      // 버퍼에 남은 데이터가 있으면 계속 전송
      if (_sendBuffer.OnSend(static_cast<int32>(bytesTransferred))) IssueSend();
    } break;

    default:
      LOG_ERR("Session::Dispatch: unknown event type (sessionIdx=%d)",
              _sessionIdx);
      break;
  }
}

void Session::ProcessPackets() {
  while (true) {
    int32 dataSize = _recvBuffer.GetDataSize();
    if (dataSize < static_cast<int32>(PACKET_HEADER_SIZE))
      break;  // 헤더도 안 왔음

    const BYTE* data = _recvBuffer.GetReadBuffer();
    const PacketHeader* header = reinterpret_cast<const PacketHeader*>(data);

    // 비정상 패킷 크기 → 연결 종료
    if (header->size < PACKET_HEADER_SIZE || header->size > SOCKET_BUF_SIZE) {
      LOG_ERR("Invalid packet size: %d (sessionIdx=%d)", header->size,
              _sessionIdx);
      Close();
      return;
    }

    if (dataSize < static_cast<int32>(header->size))
      break;  // 패킷 전체가 아직 안 옴

    OnRecv(const_cast<BYTE*>(data), static_cast<int32>(header->size));
    _recvBuffer.OnRead(static_cast<int32>(header->size));
  }
}
