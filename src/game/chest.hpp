#ifndef BTY_GAME_CHEST_HPP_
#define BTY_GAME_CHEST_HPP_

#include "engine/dialog-def.hpp"

namespace bty {
class DialogStack;
}    // namespace bty

class Hud;
struct Variables;
struct GenVariables;

void chest_roll(Variables &v, GenVariables &gen, bty::DialogStack &ds, Hud &hud);

#endif    // BTY_GAME_CHEST_HPP_
