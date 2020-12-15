#ifndef BTY_GAME_CHEST_HPP_
#define BTY_GAME_CHEST_HPP_

#include "engine/dialog-def.hpp"

namespace bty {
class DialogStack;
}

struct Variables;
struct GenVariables;

void chest_roll(Variables &v, GenVariables &gen, bty::DialogStack &ds);

#endif    // BTY_GAME_CHEST_HPP_
