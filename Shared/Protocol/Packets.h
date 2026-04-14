#pragma once
#include "Protocol/PacketHeader.h"
#include "Core/Types.h"
#include <vector>
#include <string>
#include <cstring>

// 패킷 직렬화 — 헤더 + body를 버퍼에 순서대로 기록
class PacketWriter
{
public:
    explicit PacketWriter(PacketId id, uint64_t authToken = 0)
    {
        // 헤더 영역 예약 (size는 나중에 기록)
        _buffer.resize(PACKET_HEADER_SIZE);

        PacketHeader* header = reinterpret_cast<PacketHeader*>(_buffer.data());
        header->id        = id;
        header->authToken = authToken;
    }

    template<typename T>
    PacketWriter& Write(const T& val)
    {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&val);
        _buffer.insert(_buffer.end(), ptr, ptr + sizeof(T));
        return *this;
    }

    PacketWriter& WriteString(const std::string& str)
    {
        uint16 len = static_cast<uint16>(str.size());
        Write(len);
        _buffer.insert(_buffer.end(), str.begin(), str.end());
        return *this;
    }

    // 최종 버퍼 반환 전 size 필드 기록
    const uint8_t* GetData()
    {
        reinterpret_cast<PacketHeader*>(_buffer.data())->size =
            static_cast<uint16>(_buffer.size());
        return _buffer.data();
    }

    uint16 GetSize() const { return static_cast<uint16>(_buffer.size()); }

private:
    std::vector<uint8_t> _buffer;
};

// 패킷 역직렬화 - bounds-check 실패 시 false 반환 (악의적 패킷 방어)
class PacketReader
{
public:
    PacketReader(const uint8_t* buffer, uint16 size)
        : _buffer(buffer), _size(size), _cursor(PACKET_HEADER_SIZE)
    {}

    const PacketHeader* GetHeader() const
    {
        return reinterpret_cast<const PacketHeader*>(_buffer);
    }

    template<typename T>
    bool Read(T& out)
    {
        if (_cursor + sizeof(T) > _size)
            return false;

        std::memcpy(&out, _buffer + _cursor, sizeof(T));
        _cursor += sizeof(T);
        return true;
    }

    bool ReadString(std::string& out)
    {
        uint16 len = 0;
        if (!Read(len))
            return false;

        if (_cursor + len > _size)
            return false;

        out.assign(reinterpret_cast<const char*>(_buffer + _cursor), len);
        _cursor += len;
        return true;
    }

    bool IsValid() const { return _cursor <= _size; }

private:
    const uint8_t* _buffer;
    uint16       _size;
    uint16       _cursor;
};
