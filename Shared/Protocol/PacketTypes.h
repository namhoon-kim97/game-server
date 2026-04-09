#pragma once
#include <cstdint>

// 패킷 ID — 시스템 구현 시 해당 범위에 추가
// 1000번대: 인증 / 2000번대: 게임 / 3000번대: 랭킹 / 9000번대: 시스템
enum class PacketId : uint16_t
{
    // 시스템
    C2S_PING = 9000,
    S2C_PONG = 9001,
};
