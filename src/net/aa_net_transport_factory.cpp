#include "net/aa_net_transport.hpp"

#if defined(__EMSCRIPTEN__)
#include "net/aa_net_transport_web.hpp"
#else
#include "net/aa_net_transport_native.hpp"
#endif

namespace aa
{

AAPtr<AANetTransport> createNetTransport()
{
#if defined(__EMSCRIPTEN__)
    return std::make_unique<AANetTransportWeb>();
#else
    return std::make_unique<AANetTransportNative>();
#endif
}

} // namespace aa
