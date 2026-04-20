#include "Network/RecvBuffer.h"
#include <cstring>

RecvBuffer::RecvBuffer(int32 bufferSize)
{
    _buffer.resize(bufferSize);
}

void RecvBuffer::Clean()
{
    int32 dataSize = GetDataSize();
    if (dataSize == 0)
    {
        // 읽을 데이터 없으면 커서만 초기화
        _readPos = _writePos = 0;
    }
    else
    {
        // 남은 데이터를 버퍼 앞으로 이동
        std::memmove(_buffer.data(), GetReadBuffer(), dataSize);
        _readPos  = 0;
        _writePos = dataSize;
    }
}
