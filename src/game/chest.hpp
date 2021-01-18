#ifndef BTY_GAME_CHEST2_HPP_
#define BTY_GAME_CHEST2_HPP_

class Chest {
public:
    virtual void load() = 0;
    virtual void show() = 0;
    virtual int getChance() = 0;
};

#endif    // BTY_GAME_CHEST2_HPP_
