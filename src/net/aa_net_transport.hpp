#pragma once

#include "core/aa_memory.hpp"
#include "core/aa_types.hpp"

#include <vector>

namespace aa
{

enum class NetConnectionStatus : u8
{
    Disconnected,
    Connecting,
    Connected,
    Error
};

class AANetTransport
{
public:
    virtual ~AANetTransport() = default;

    virtual void connect(const char* url) = 0;
    virtual void disconnect() = 0;
    virtual void send(const u8* data, usize size) = 0;
    virtual void poll() = 0;

    virtual NetConnectionStatus status() const = 0;

    virtual b8 hasMessage() const = 0;
    virtual std::vector<std::vector<u8>>& pendingMessages() = 0;
};

AAPtr<AANetTransport> createNetTransport();

} // namespace aa
