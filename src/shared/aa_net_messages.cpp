#include "shared/aa_net_messages.hpp"

#include <cstring>

static_assert(sizeof(float) == 4, "float must be 4 bytes");

namespace aa
{

// -- Write helpers --

static void writeU8(std::vector<u8>& buf, u8 v) { buf.push_back(v); }

static void writeU16(std::vector<u8>& buf, u16 v)
{
    buf.push_back(static_cast<u8>(v & 0xFF));
    buf.push_back(static_cast<u8>((v >> 8) & 0xFF));
}

static void writeF32(std::vector<u8>& buf, f32 v)
{
    u8 bytes[4];
    std::memcpy(bytes, &v, 4);
    buf.insert(buf.end(), bytes, bytes + 4);
}

// -- Read helpers --

static u8 readU8(const u8* data, usize& offset)
{
    return data[offset++];
}

static u16 readU16(const u8* data, usize& offset)
{
    u16 v = static_cast<u16>(data[offset]) | (static_cast<u16>(data[offset + 1]) << 8);
    offset += 2;
    return v;
}

static f32 readF32(const u8* data, usize& offset)
{
    f32 v;
    std::memcpy(&v, data + offset, 4);
    offset += 4;
    return v;
}

// -- Serialize Client Messages --

void serializeClientHello(std::vector<u8>& out)
{
    out.clear();
    writeU8(out, static_cast<u8>(NetMessageType::ClientHello));
    writeU8(out, kNetProtocolVersion);
}

void serializeClientInput(std::vector<u8>& out, u8 inputBits)
{
    out.clear();
    writeU8(out, static_cast<u8>(NetMessageType::ClientInput));
    writeU8(out, inputBits);
}

void serializeClientBye(std::vector<u8>& out)
{
    out.clear();
    writeU8(out, static_cast<u8>(NetMessageType::ClientBye));
}

// -- Serialize Server Messages --

void serializeServerWelcome(std::vector<u8>& out, u8 playerId)
{
    out.clear();
    writeU8(out, static_cast<u8>(NetMessageType::ServerWelcome));
    writeU8(out, kNetProtocolVersion);
    writeU8(out, playerId);
    writeU8(out, 0); // reserved
    writeU16(out, kServerTickIntervalMs);
}

void serializeServerState(std::vector<u8>& out, const GameState& state)
{
    out.clear();
    writeU8(out, static_cast<u8>(NetMessageType::ServerState));
    writeU8(out, static_cast<u8>(state.players.size()));

    for (const auto& p : state.players)
    {
        writeU8(out, p.playerId);
        writeF32(out, p.posX);
        writeF32(out, p.posY);
        writeU8(out, p.colorR);
        writeU8(out, p.colorG);
        writeU8(out, p.colorB);
        writeU8(out, p.inputBits);
    }
}

void serializeServerBye(std::vector<u8>& out)
{
    out.clear();
    writeU8(out, static_cast<u8>(NetMessageType::ServerBye));
}

// -- Peek --

NetMessageType peekMessageType(const u8* data, usize size)
{
    if (size < 1)
    {
        return static_cast<NetMessageType>(0);
    }
    return static_cast<NetMessageType>(data[0]);
}

// -- Deserialize Client Messages --

b8 deserializeClientHello(const u8* data, usize size, u8& outProtocolVersion)
{
    if (size < 2)
    {
        return false;
    }
    outProtocolVersion = data[1];
    return true;
}

b8 deserializeClientInput(const u8* data, usize size, u8& outInputBits)
{
    if (size < 2)
    {
        return false;
    }
    outInputBits = data[1];
    return true;
}

// -- Deserialize Server Messages --

b8 deserializeServerWelcome(const u8* data, usize size, u8& outPlayerId, u16& outTickRateMs)
{
    if (size < 6)
    {
        return false;
    }
    usize offset = 2; // skip type + version
    outPlayerId = readU8(data, offset);
    offset++; // skip reserved
    outTickRateMs = readU16(data, offset);
    return true;
}

b8 deserializeServerState(const u8* data, usize size, GameState& outState)
{
    if (size < 2)
    {
        return false;
    }

    usize offset = 1;
    u8 playerCount = readU8(data, offset);

    usize requiredSize = 2 + static_cast<usize>(playerCount) * 13;
    if (size < requiredSize)
    {
        return false;
    }

    outState.players.resize(playerCount);
    for (u8 i = 0; i < playerCount; ++i)
    {
        auto& p = outState.players[i];
        p.playerId = readU8(data, offset);
        p.posX = readF32(data, offset);
        p.posY = readF32(data, offset);
        p.colorR = readU8(data, offset);
        p.colorG = readU8(data, offset);
        p.colorB = readU8(data, offset);
        p.inputBits = readU8(data, offset);
    }
    return true;
}

} // namespace aa
