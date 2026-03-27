#include "shared/aa_game_sim.hpp"
#include "shared/aa_net_protocol.hpp"

namespace aa
{

void simulatePlayerMovement(PlayerState& player, u8 inputBits, f32 deltaTime)
{
    f32 dx = 0.0f;
    f32 dy = 0.0f;

    if (inputBits & kInputUp) dy -= 1.0f;
    if (inputBits & kInputDown) dy += 1.0f;
    if (inputBits & kInputLeft) dx -= 1.0f;
    if (inputBits & kInputRight) dx += 1.0f;

    // Normalize diagonal movement
    if (dx != 0.0f && dy != 0.0f)
    {
        constexpr f32 invSqrt2 = 0.70710678f;
        dx *= invSqrt2;
        dy *= invSqrt2;
    }

    player.posX += dx * kPlayerMoveSpeed * deltaTime;
    player.posY += dy * kPlayerMoveSpeed * deltaTime;

    // Clamp to arena bounds
    if (player.posX < 0.0f) player.posX = 0.0f;
    if (player.posY < 0.0f) player.posY = 0.0f;
    if (player.posX > kArenaWidth - kPlayerBoxWidth) player.posX = kArenaWidth - kPlayerBoxWidth;
    if (player.posY > kArenaHeight - kPlayerBoxHeight) player.posY = kArenaHeight - kPlayerBoxHeight;
}

void assignPlayerColor(PlayerState& player)
{
    static const u8 colors[][3] = {
        {231, 76, 60},   // red
        {46, 204, 113},  // green
        {52, 152, 219},  // blue
        {241, 196, 15},  // yellow
        {155, 89, 182},  // purple
        {230, 126, 34},  // orange
        {26, 188, 156},  // teal
        {236, 240, 241}, // white
        {192, 57, 43},   // dark red
        {39, 174, 96},   // dark green
        {41, 128, 185},  // dark blue
        {243, 156, 18},  // dark yellow
        {142, 68, 173},  // dark purple
        {211, 84, 0},    // dark orange
        {22, 160, 133},  // dark teal
        {149, 165, 166}, // gray
    };

    u8 idx = player.playerId % 16;
    player.colorR = colors[idx][0];
    player.colorG = colors[idx][1];
    player.colorB = colors[idx][2];
}

} // namespace aa
