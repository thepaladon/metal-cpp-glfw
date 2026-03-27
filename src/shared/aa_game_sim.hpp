#pragma once

#include "shared/aa_net_messages.hpp"

namespace aa
{

void simulatePlayerMovement(PlayerState& player, u8 inputBits, f32 deltaTime);
void assignPlayerColor(PlayerState& player);

} // namespace aa
