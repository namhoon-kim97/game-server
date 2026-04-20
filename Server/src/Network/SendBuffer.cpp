#include "Network/SendBuffer.h"
#include <cstring>

SendBuffer::SendBuffer(int32 bufferSize)
{
    _buffer.resize(bufferSize);
}

bool SendBuffer::Push(const BYTE* data, int32 len)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_dataSize + len > static_cast<int32>(_buffer.size()))
        return false; // 버퍼 초과 - 상위에서 연결 끊기 처리

    std::memcpy(_buffer.data() + _dataSize, data, len);
    _dataSize += len;

    if (_pending)
        return false; // 이미 WSASend pending 중 - 완료 후 OnSend에서 재전송

    _pending = true;
    return true; // 호출자가 WSASend 등록해야 함
}

bool SendBuffer::OnSend(int32 numOfBytes)
{
    std::lock_guard<std::mutex> lock(_mutex);

    std::memmove(_buffer.data(), _buffer.data() + numOfBytes, _dataSize - numOfBytes);
    _dataSize -= numOfBytes;

    if (_dataSize == 0)
    {
        _pending = false;
        return false; // 보낼 데이터 없음
    }

    return true; // 남은 데이터 있음 - 호출자가 WSASend 재등록
}
