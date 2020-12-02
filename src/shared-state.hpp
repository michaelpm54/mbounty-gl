#ifndef BTY_SHARED_STATE_HPP_
#define BTY_SHARED_STATE_HPP_

#include <vector>

struct SharedState {
    int hero_id{0};
    int hero_rank{0};
    int difficulty_level{2};
    int days{0};
    int continent{0};
    int army[5]{-1};
    int army_counts[5]{0};
    int army_morales[5]{0};
    int leadership{0};
    int contract{17};
    bool siege{false};
    bool magic{false};
    int x{0};
    int y{0};
    std::vector<uint8_t> visited_tiles;
    int spells[14];
    int gold{0};
    int commission{0};
    int max_spells{0};
    int spell_power{0};
};

#endif // BTY_SHARED_STATE_HPP_
