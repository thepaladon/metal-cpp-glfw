#include "game/aa_game_client.hpp"
#include "shared/aa_net_protocol.hpp"

#include "imgui.h"

namespace aa
{

void AAGameClient::initialize(const char* serverUrl)
{
    serverUrl_ = serverUrl;
    transport_ = createNetTransport();
    transport_->connect(serverUrl_);

    serializeClientHello(sendBuffer_);
}

void AAGameClient::shutdown()
{
    if (transport_)
    {
        serializeClientBye(sendBuffer_);
        transport_->send(sendBuffer_.data(), sendBuffer_.size());
        transport_->disconnect();
        transport_.reset();
    }
}

void AAGameClient::update(f32 deltaTime)
{
    if (!transport_)
    {
        return;
    }

    transport_->poll();

    auto connStatus = transport_->status();

    if (connStatus == NetConnectionStatus::Connected && !welcomed_)
    {
        // Send hello on first connect
        serializeClientHello(sendBuffer_);
        transport_->send(sendBuffer_.data(), sendBuffer_.size());
    }

    if (connStatus == NetConnectionStatus::Disconnected ||
        connStatus == NetConnectionStatus::Error)
    {
        welcomed_ = false;
        localPlayerId_ = 0;
        gameState_.players.clear();
        tryReconnect(deltaTime);
        return;
    }

    processIncomingMessages();

    if (welcomed_)
    {
        // Read input from ImGui's key state
        currentInputBits_ = 0;
        if (ImGui::IsKeyDown(ImGuiKey_W)) currentInputBits_ |= kInputUp;
        if (ImGui::IsKeyDown(ImGuiKey_A)) currentInputBits_ |= kInputLeft;
        if (ImGui::IsKeyDown(ImGuiKey_S)) currentInputBits_ |= kInputDown;
        if (ImGui::IsKeyDown(ImGuiKey_D)) currentInputBits_ |= kInputRight;

        sendInput();
    }
}

void AAGameClient::renderImGui()
{
    ImGui::Begin("Game Arena", nullptr,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 contentMin = ImGui::GetCursorScreenPos();
    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw arena background
    ImVec2 arenaMax(contentMin.x + contentSize.x, contentMin.y + contentSize.y);
    drawList->AddRectFilled(contentMin, arenaMax, IM_COL32(30, 30, 30, 255));
    drawList->AddRect(contentMin, arenaMax, IM_COL32(80, 80, 80, 255));

    // Scale factor to fit arena into available space
    f32 scaleX = contentSize.x / kArenaWidth;
    f32 scaleY = contentSize.y / kArenaHeight;
    f32 scale = (scaleX < scaleY) ? scaleX : scaleY;

    // Center the arena
    f32 offsetX = contentMin.x + (contentSize.x - kArenaWidth * scale) * 0.5f;
    f32 offsetY = contentMin.y + (contentSize.y - kArenaHeight * scale) * 0.5f;

    for (const auto& player : gameState_.players)
    {
        f32 px = offsetX + player.posX * scale;
        f32 py = offsetY + player.posY * scale;
        f32 pw = kPlayerBoxWidth * scale;
        f32 ph = kPlayerBoxHeight * scale;

        ImVec2 pMin(px, py);
        ImVec2 pMax(px + pw, py + ph);

        ImU32 color = IM_COL32(player.colorR, player.colorG, player.colorB, 255);
        drawList->AddRectFilled(pMin, pMax, color);

        // Highlight local player with a white border
        if (player.playerId == localPlayerId_)
        {
            drawList->AddRect(pMin, pMax, IM_COL32(255, 255, 255, 255), 0.0f, 0, 2.0f);
        }

        // Player ID label
        char label[16];
        snprintf(label, sizeof(label), "P%u", player.playerId);
        ImVec2 textPos(px + 2.0f, py + 2.0f);
        drawList->AddText(textPos, IM_COL32(0, 0, 0, 255), label);
    }

    ImGui::End();

    // Connection status overlay
    ImGui::Begin("Network Status", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

    if (!transport_)
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "No transport");
    }
    else
    {
        auto s = transport_->status();
        if (s == NetConnectionStatus::Connected && welcomed_)
        {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Connected (Player %u)", localPlayerId_);
        }
        else if (s == NetConnectionStatus::Connected)
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Handshaking...");
        }
        else if (s == NetConnectionStatus::Connecting)
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Connecting...");
        }
        else
        {
            ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Disconnected (reconnecting...)");
        }
    }

    ImGui::Text("Players: %zu", gameState_.players.size());
    ImGui::Text("WASD to move");
    ImGui::End();
}

b8 AAGameClient::isConnected() const
{
    return transport_ && transport_->status() == NetConnectionStatus::Connected && welcomed_;
}

void AAGameClient::processIncomingMessages()
{
    auto& messages = transport_->pendingMessages();
    for (auto& msg : messages)
    {
        if (msg.empty())
        {
            continue;
        }

        auto type = peekMessageType(msg.data(), msg.size());

        if (type == NetMessageType::ServerWelcome)
        {
            u16 tickRateMs = 0;
            if (deserializeServerWelcome(msg.data(), msg.size(), localPlayerId_, tickRateMs))
            {
                welcomed_ = true;
            }
        }
        else if (type == NetMessageType::ServerState)
        {
            deserializeServerState(msg.data(), msg.size(), gameState_);
        }
        else if (type == NetMessageType::ServerBye)
        {
            transport_->disconnect();
            welcomed_ = false;
        }
    }
    messages.clear();
}

void AAGameClient::sendInput()
{
    serializeClientInput(sendBuffer_, currentInputBits_);
    transport_->send(sendBuffer_.data(), sendBuffer_.size());
}

void AAGameClient::tryReconnect(f32 deltaTime)
{
    reconnectTimer_ += deltaTime;
    if (reconnectTimer_ >= kReconnectInterval)
    {
        reconnectTimer_ = 0.0f;
        if (transport_ && serverUrl_)
        {
            transport_->connect(serverUrl_);
        }
    }
}

} // namespace aa
