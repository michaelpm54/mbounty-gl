#ifndef BTY_DATA_CASTLES_HPP_
#define BTY_DATA_CASTLES_HPP_

struct CastleInfo {
    std::string name;
    int x;
    int y;
    int continent;
};

inline const CastleInfo kCastleInfo[] = {
    {"Azram", 30, 27, 0},
    {"Basefit", 47, 6, 1},
    {"Cancomar", 36, 49, 0},
    {"Duvock", 30, 18, 1},
    {"Endryx", 11, 46, 3},
    {"Faxis", 22, 49, 0},
    {"Goobare", 41, 36, 2},
    {"Hyppus", 43, 27, 2},
    {"Irok", 11, 30, 0},
    {"Jhan", 41, 34, 1},
    {"Kookamunga", 57, 58, 0},
    {"Lorshe", 52, 57, 2},
    {"Mooseweigh", 25, 39, 1},
    {"Nilslag", 22, 24, 0},
    {"Ophiraund", 6, 57, 0},
    {"Portalis", 58, 23, 0},
    {"Quinderwitch", 42, 56, 1},
    {"Rythacon", 54, 6, 0},
    {"Spockana", 17, 39, 3},
    {"Tylitch", 9, 18, 2},
    {"Uzare", 41, 12, 3},
    {"Vutar", 40, 5, 0},
    {"Wankelforte", 40, 41, 0},
    {"Xelox", 45, 6, 2},
    {"Yeneverre", 19, 19, 1},
    {"Zyzzarzaz", 46, 43, 3},
};

inline constexpr int const kCastlesPerContinent[4] = {
    11,
    6,
    6,
    3,
};

inline constexpr int const kCastleIndices[4] = {
    0,
    11,
    17,
    23,
};

#endif    // BTY_DATA_CASTLES_HPP_
