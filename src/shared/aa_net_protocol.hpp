#pragma once

#include "core/aa_types.hpp"

namespace aa
{

constexpr u8 kNetProtocolVersion = 1;

constexpr f32 kServerTickRate = 20.0f;
constexpr f32 kServerTickInterval = 1.0f / 20.0f;
constexpr u16 kServerTickIntervalMs = 50;

constexpr f32 kPlayerMoveSpeed = 200.0f;

constexpr u32 kMaxPlayers = 16;

constexpr f32 kPlayerBoxWidth = 40.0f;
constexpr f32 kPlayerBoxHeight = 40.0f;

constexpr f32 kArenaWidth = 1200.0f;
constexpr f32 kArenaHeight = 680.0f;

constexpr u16 kServerPort = 9001;

constexpr u8 kInputUp = 1 << 0;
constexpr u8 kInputLeft = 1 << 1;
constexpr u8 kInputDown = 1 << 2;
constexpr u8 kInputRight = 1 << 3;

enum class NetMessageType : u8
{
    ClientHello = 0x01,
    ClientInput = 0x02,
    ClientBye = 0x03,

    ServerWelcome = 0x10,
    ServerState = 0x11,
    ServerBye = 0x12,
};

} // namespace aa
