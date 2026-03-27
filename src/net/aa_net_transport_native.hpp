#pragma once

#if !defined(__EMSCRIPTEN__)

#include "net/aa_net_transport.hpp"

namespace aa
{

// Stub native transport for MVP.
// The primary client path is via browser (Emscripten WebSocket).
// Native WebSocket client support can be added later with a dedicated library.
class AANetTransportNative final : public AANetTransport
{
public:
    void connect(const char* url) override;
    void disconnect() override;
    void send(const u8* data, usize size) override;
    void poll() override;

    NetConnectionStatus status() const override;
    b8 hasMessage() const override;
    std::vector<std::vector<u8>>& pendingMessages() override;

private:
    NetConnectionStatus status_ = NetConnectionStatus::Disconnected;
    std::vector<std::vector<u8>> incoming_;
};

} // namespace aa

#endif // !__EMSCRIPTEN__
