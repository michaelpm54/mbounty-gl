#ifndef BTY_GAME_CHEST_HPP_
#define BTY_GAME_CHEST_HPP_

#include "game/dialog-def.hpp"

struct SharedState;

void chest_roll(SharedState &state, std::function<void(const DialogDef &)> show_dialog);

#endif    // BTY_GAME_CHEST_HPP_
