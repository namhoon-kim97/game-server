#pragma once
#include <cstdint>

using BYTE   = unsigned char;

using int16  = int16_t;
using uint16 = uint16_t;
using int32  = int32_t;
using uint32 = uint32_t;
using int64  = int64_t;
using uint64 = uint64_t;

constexpr int32  MAX_BUFFER      = 1024;
constexpr int32  MAX_EVENTS      = 64;
constexpr int32  SOCKET_BUF_SIZE = (128 * 1024);
constexpr int32  MIN_RECV_SIZE   = 4096;
constexpr int32  MAX_SESSION     = 20000;

constexpr uint16 MIN_PACKETCODE  = 0x0001;
constexpr uint16 MAX_PACKETCODE  = 0xfffe;

enum class EServerType : int32 { NONE = 0, DB_AGENT = 1, LOGIN_SERVER = 2 };
