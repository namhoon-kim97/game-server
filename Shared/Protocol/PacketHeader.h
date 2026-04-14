#pragma once
#include "Core/Types.h"
#include "Protocol/PacketTypes.h"

// padding 없이 네트워크 전송 크기와 구조체 크기를 일치시키기 위해 1바이트 정렬
#pragma pack(push, 1)

struct PacketHeader
{
    uint16  size;       // 헤더 포함 전체 패킷 크기
    PacketId  id;       // 패킷 종류
    uint64  authToken;  // 인증 토큰 (0 = 미인증)
};

#pragma pack(pop)

static constexpr size_t PACKET_HEADER_SIZE = sizeof(PacketHeader); // 12바이트
