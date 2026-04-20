#pragma once
#include "Core/Types.h"
#include <vector>
#include <mutex>

// 송신 버퍼
// WSASend가 pending 중이면 버퍼에만 쌓고, 완료 시 남은 데이터 재전송
class SendBuffer
{
public:
    explicit SendBuffer(int32 bufferSize = SOCKET_BUF_SIZE);

    // true: WSASend 바로 등록 필요 / false: 이미 pending 중, 버퍼에만 추가됨
    bool Push(const BYTE* data, int32 len);

    // WSASend에 넘길 포인터와 크기
    BYTE*  GetSendBuffer() { return _buffer.data(); }
    int32  GetDataSize()   { return _dataSize; }

    // WSASend 완료 시 호출 - 전송된 만큼 제거, 남은 데이터 있으면 true 반환
    bool OnSend(int32 numOfBytes);

    bool IsPending() const { return _pending; }

private:
    std::vector<BYTE> _buffer;
    int32             _dataSize = 0;
    bool              _pending  = false;
    std::mutex        _mutex;
};
