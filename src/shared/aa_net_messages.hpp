#pragma once

#include "core/aa_types.hpp"
#include "shared/aa_net_protocol.hpp"

#include <vector>

namespace aa
{

struct PlayerState
{
    u8 playerId = 0;
    f32 posX = 0.0f;
    f32 posY = 0.0f;
    u8 colorR = 255;
    u8 colorG = 255;
    u8 colorB = 255;
    u8 inputBits = 0;
};

struct GameState
{
    std::vector<PlayerState> players;
};

// Serialization — writes complete messages into a buffer
void serializeClientHello(std::vector<u8>& out);
void serializeClientInput(std::vector<u8>& out, u8 inputBits);
void serializeClientBye(std::vector<u8>& out);

void serializeServerWelcome(std::vector<u8>& out, u8 playerId);
void serializeServerState(std::vector<u8>& out, const GameState& state);
void serializeServerBye(std::vector<u8>& out);

// Peek at the message type from raw data
NetMessageType peekMessageType(const u8* data, usize size);

// Deserialization — returns false on parse failure
b8 deserializeClientHello(const u8* data, usize size, u8& outProtocolVersion);
b8 deserializeClientInput(const u8* data, usize size, u8& outInputBits);
b8 deserializeServerWelcome(const u8* data, usize size, u8& outPlayerId, u16& outTickRateMs);
b8 deserializeServerState(const u8* data, usize size, GameState& outState);

} // namespace aa
