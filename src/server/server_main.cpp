#include "shared/aa_game_sim.hpp"
#include "shared/aa_net_messages.hpp"
#include "shared/aa_net_protocol.hpp"

#include <App.h>

#include <cstdio>
#include <map>
#include <vector>

using namespace aa;

struct ConnectionData
{
    u8 playerId = 0;
};

static GameState gGameState;
static std::map<void*, u8> gSocketToPlayer;
static std::vector<u8> gSendBuffer;

static u8 allocatePlayerId()
{
    for (u8 id = 1; id != 0; ++id)
    {
        bool taken = false;
        for (const auto& p : gGameState.players)
        {
            if (p.playerId == id) { taken = true; break; }
        }
        if (!taken) return id;
    }
    return 0;
}

// Store websockets for broadcasting
static std::vector<uWS::WebSocket<false, true, ConnectionData>*> gClients;

void onTick()
{
    for (auto& player : gGameState.players)
    {
        simulatePlayerMovement(player, player.inputBits, kServerTickInterval);
    }

    serializeServerState(gSendBuffer, gGameState);
    std::string_view msg(reinterpret_cast<const char*>(gSendBuffer.data()), gSendBuffer.size());

    for (auto* ws : gClients)
    {
        ws->send(msg, uWS::OpCode::BINARY);
    }
}

int main()
{
    std::printf("Starting game server on port %u at %.0f Hz...\n", kServerPort, kServerTickRate);

    uWS::App app;

    app.ws<ConnectionData>("/*", {
        .compression = uWS::DISABLED,
        .maxPayloadLength = 1024,
        .idleTimeout = 120,
        .maxBackpressure = 64 * 1024,

        .open = [](auto* ws) {
            auto* data = ws->getUserData();
            data->playerId = allocatePlayerId();

            PlayerState player;
            player.playerId = data->playerId;
            player.posX = kArenaWidth * 0.5f;
            player.posY = kArenaHeight * 0.5f;
            assignPlayerColor(player);
            gGameState.players.push_back(player);

            gClients.push_back(ws);

            std::vector<u8> welcomeBuf;
            serializeServerWelcome(welcomeBuf, data->playerId);
            std::string_view msg(reinterpret_cast<const char*>(welcomeBuf.data()), welcomeBuf.size());
            ws->send(msg, uWS::OpCode::BINARY);

            std::printf("Player %u connected (%zu total)\n",
                data->playerId, gGameState.players.size());
        },

        .message = [](auto* ws, std::string_view message, uWS::OpCode opCode) {
            if (opCode != uWS::OpCode::BINARY || message.empty())
            {
                return;
            }

            const u8* msgData = reinterpret_cast<const u8*>(message.data());
            usize msgSize = message.size();
            auto type = peekMessageType(msgData, msgSize);

            if (type == NetMessageType::ClientInput)
            {
                u8 inputBits = 0;
                if (deserializeClientInput(msgData, msgSize, inputBits))
                {
                    auto* data = ws->getUserData();
                    for (auto& p : gGameState.players)
                    {
                        if (p.playerId == data->playerId)
                        {
                            p.inputBits = inputBits;
                            break;
                        }
                    }
                }
            }
            else if (type == NetMessageType::ClientBye)
            {
                ws->close();
            }
        },

        .close = [](auto* ws, int /*code*/, std::string_view /*message*/) {
            auto* data = ws->getUserData();
            u8 pid = data->playerId;

            auto& players = gGameState.players;
            for (auto it = players.begin(); it != players.end(); ++it)
            {
                if (it->playerId == pid)
                {
                    players.erase(it);
                    break;
                }
            }

            for (auto it = gClients.begin(); it != gClients.end(); ++it)
            {
                if (*it == ws)
                {
                    gClients.erase(it);
                    break;
                }
            }

            std::printf("Player %u disconnected (%zu remaining)\n",
                pid, gGameState.players.size());
        }
    });

    app.listen(kServerPort, [](auto* listenSocket) {
        if (listenSocket)
        {
            std::printf("Listening on port %u\n", kServerPort);
        }
        else
        {
            std::fprintf(stderr, "Failed to listen on port %u\n", kServerPort);
        }
    });

    // Set up a repeating timer for the game tick
    struct us_loop_t* loop = reinterpret_cast<struct us_loop_t*>(uWS::Loop::get());
    struct us_timer_t* tickTimer = us_create_timer(loop, 0, 0);
    us_timer_set(tickTimer, [](struct us_timer_t* /*timer*/) {
        onTick();
    }, static_cast<int>(kServerTickIntervalMs), static_cast<int>(kServerTickIntervalMs));

    app.run();
    return 0;
}
