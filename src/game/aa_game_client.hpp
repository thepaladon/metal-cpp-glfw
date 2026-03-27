#pragma once

#include "core/aa_memory.hpp"
#include "core/aa_types.hpp"
#include "net/aa_net_transport.hpp"
#include "shared/aa_net_messages.hpp"

#include <vector>

namespace aa
{

class AAGameClient
{
public:
    void initialize(const char* serverUrl);
    void shutdown();
    void update(f32 deltaTime);
    void renderImGui();

    b8 isConnected() const;

private:
    void processIncomingMessages();
    void sendInput();
    void tryReconnect(f32 deltaTime);

    AAPtr<AANetTransport> transport_;
    GameState gameState_;
    u8 localPlayerId_ = 0;
    u8 currentInputBits_ = 0;
    b8 welcomed_ = false;

    std::vector<u8> sendBuffer_;

    // Reconnection
    f32 reconnectTimer_ = 0.0f;
    static constexpr f32 kReconnectInterval = 3.0f;
    const char* serverUrl_ = nullptr;
};

} // namespace aa
