#pragma once

#if defined(__EMSCRIPTEN__)

#include "net/aa_net_transport.hpp"

namespace aa
{

class AANetTransportWeb final : public AANetTransport
{
public:
    AANetTransportWeb();
    ~AANetTransportWeb() override;

    void connect(const char* url) override;
    void disconnect() override;
    void send(const u8* data, usize size) override;
    void poll() override;

    NetConnectionStatus status() const override;
    b8 hasMessage() const override;
    std::vector<std::vector<u8>>& pendingMessages() override;

    // Called from JS callbacks
    void onOpen();
    void onMessage(const u8* data, usize size);
    void onClose();
    void onError();

private:
    int socketHandle_ = -1;
    NetConnectionStatus status_ = NetConnectionStatus::Disconnected;
    std::vector<std::vector<u8>> incoming_;
};

} // namespace aa

#endif // __EMSCRIPTEN__
