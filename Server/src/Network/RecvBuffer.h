#pragma once
#include "Core/Types.h"
#include <vector>

// 수신 버퍼
class RecvBuffer
{
public:
    explicit RecvBuffer(int32 bufferSize = MIN_RECV_SIZE);

    // WSARecv에 넘길 버퍼 포인터와 가용 크기
    BYTE*  GetWriteBuffer() { return _buffer.data() + _writePos; }
    int32  GetFreeSize()    { return static_cast<int32>(_buffer.size()) - _writePos; }

    // WSARecv 완료 시 수신된 바이트만큼 호출
    void OnWrite(int32 numOfBytes) { _writePos += numOfBytes; }

    // 패킷 파싱에 사용할 버퍼 포인터와 누적 데이터 크기
    BYTE*  GetReadBuffer() { return _buffer.data() + _readPos; }
    int32  GetDataSize()   { return _writePos - _readPos; }

    // 패킷 소비 후 읽은 바이트만큼 호출
    void OnRead(int32 numOfBytes) { _readPos += numOfBytes; }

    // 남은 데이터를 버퍼 앞으로 당겨 여유 공간 확보
    void Clean();

private:
    std::vector<BYTE> _buffer;
    int32             _readPos  = 0;
    int32             _writePos = 0;
};
