#if defined(__EMSCRIPTEN__)

#include "net/aa_net_transport_web.hpp"

#include <emscripten/websocket.h>

namespace aa
{

// Static callbacks that forward to the transport instance
static EM_BOOL wsOnOpen(int /*eventType*/, const EmscriptenWebSocketOpenEvent* /*event*/, void* userData)
{
    auto* self = static_cast<AANetTransportWeb*>(userData);
    self->onOpen();
    return EM_TRUE;
}

static EM_BOOL wsOnMessage(int /*eventType*/, const EmscriptenWebSocketMessageEvent* event, void* userData)
{
    auto* self = static_cast<AANetTransportWeb*>(userData);
    if (event->isText == 0 && event->numBytes > 0)
    {
        self->onMessage(event->data, static_cast<usize>(event->numBytes));
    }
    return EM_TRUE;
}

static EM_BOOL wsOnClose(int /*eventType*/, const EmscriptenWebSocketCloseEvent* /*event*/, void* userData)
{
    auto* self = static_cast<AANetTransportWeb*>(userData);
    self->onClose();
    return EM_TRUE;
}

static EM_BOOL wsOnError(int /*eventType*/, const EmscriptenWebSocketErrorEvent* /*event*/, void* userData)
{
    auto* self = static_cast<AANetTransportWeb*>(userData);
    self->onError();
    return EM_TRUE;
}

AANetTransportWeb::AANetTransportWeb() = default;

AANetTransportWeb::~AANetTransportWeb()
{
    disconnect();
}

void AANetTransportWeb::connect(const char* url)
{
    if (socketHandle_ >= 0)
    {
        disconnect();
    }

    EmscriptenWebSocketCreateAttributes attrs;
    attrs.url = url;
    attrs.protocols = nullptr;
    attrs.createOnMainThread = EM_TRUE;

    socketHandle_ = emscripten_websocket_new(&attrs);
    if (socketHandle_ < 0)
    {
        status_ = NetConnectionStatus::Error;
        return;
    }

    status_ = NetConnectionStatus::Connecting;

    emscripten_websocket_set_onopen_callback(socketHandle_, this, wsOnOpen);
    emscripten_websocket_set_onmessage_callback(socketHandle_, this, wsOnMessage);
    emscripten_websocket_set_onclose_callback(socketHandle_, this, wsOnClose);
    emscripten_websocket_set_onerror_callback(socketHandle_, this, wsOnError);
}

void AANetTransportWeb::disconnect()
{
    if (socketHandle_ >= 0)
    {
        emscripten_websocket_close(socketHandle_, 1000, "bye");
        emscripten_websocket_delete(socketHandle_);
        socketHandle_ = -1;
    }
    status_ = NetConnectionStatus::Disconnected;
}

void AANetTransportWeb::send(const u8* data, usize size)
{
    if (socketHandle_ >= 0 && status_ == NetConnectionStatus::Connected)
    {
        emscripten_websocket_send_binary(socketHandle_, const_cast<void*>(static_cast<const void*>(data)),
            static_cast<uint32_t>(size));
    }
}

void AANetTransportWeb::poll()
{
    // Emscripten callbacks fire on the main thread automatically
}

NetConnectionStatus AANetTransportWeb::status() const
{
    return status_;
}

b8 AANetTransportWeb::hasMessage() const
{
    return !incoming_.empty();
}

std::vector<std::vector<u8>>& AANetTransportWeb::pendingMessages()
{
    return incoming_;
}

void AANetTransportWeb::onOpen()
{
    status_ = NetConnectionStatus::Connected;
}

void AANetTransportWeb::onMessage(const u8* data, usize size)
{
    incoming_.emplace_back(data, data + size);
}

void AANetTransportWeb::onClose()
{
    status_ = NetConnectionStatus::Disconnected;
    socketHandle_ = -1;
}

void AANetTransportWeb::onError()
{
    status_ = NetConnectionStatus::Error;
}

} // namespace aa

#endif // __EMSCRIPTEN__
