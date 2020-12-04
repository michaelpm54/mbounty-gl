#ifndef BTY_GAME_MOVE_FLAGS_HPP_
#define BTY_GAME_MOVE_FLAGS_HPP_

enum MoveFlags {
    MOVE_FLAGS_NONE = 0,
    MOVE_FLAGS_LEFT = 1 << 0,
    MOVE_FLAGS_RIGHT = 1 << 1,
    MOVE_FLAGS_UP = 1 << 2,
    MOVE_FLAGS_DOWN = 1 << 3,
};

#endif    // BTY_GAME_MOVE_FLAGS_HPP_
