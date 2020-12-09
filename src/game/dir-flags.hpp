#ifndef BTY_GAME_MOVE_FLAGS_HPP_
#define BTY_GAME_MOVE_FLAGS_HPP_

enum DirFlags {
    DIR_FLAG_NONE = 0,
    DIR_FLAG_LEFT = 1 << 0,
    DIR_FLAG_RIGHT = 1 << 1,
    DIR_FLAG_UP = 1 << 2,
    DIR_FLAG_DOWN = 1 << 3,
};

#endif    // BTY_GAME_MOVE_FLAGS_HPP_
