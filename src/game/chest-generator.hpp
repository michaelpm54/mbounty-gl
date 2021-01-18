#ifndef BTY_GAME_CHEST_GENERATOR_HPP
#define BTY_GAME_CHEST_GENERATOR_HPP

#include <vector>

class Chest;

class ChestGenerator {
public:
    void addChest(Chest &chest, bool fallback = false);
    void roll();

private:
    std::vector<Chest *> _chests;
    Chest *_fallback {nullptr};
};

#endif    // BTY_GAME_CHEST_GENERATOR_HPP
