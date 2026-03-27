#if !defined(__EMSCRIPTEN__)

#include "net/aa_net_transport_native.hpp"

#include <cstdio>

namespace aa
{

void AANetTransportNative::connect(const char* url)
{
    std::printf("[AANet] Native WebSocket client not yet implemented. URL: %s\n", url);
    std::printf("[AANet] Use the browser (Emscripten) build to connect to the server.\n");
    status_ = NetConnectionStatus::Disconnected;
}

void AANetTransportNative::disconnect()
{
    status_ = NetConnectionStatus::Disconnected;
}

void AANetTransportNative::send(const u8* /*data*/, usize /*size*/)
{
}

void AANetTransportNative::poll()
{
}

NetConnectionStatus AANetTransportNative::status() const
{
    return status_;
}

b8 AANetTransportNative::hasMessage() const
{
    return false;
}

std::vector<std::vector<u8>>& AANetTransportNative::pendingMessages()
{
    return incoming_;
}

} // namespace aa

#endif // !__EMSCRIPTEN__
