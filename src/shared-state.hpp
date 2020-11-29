#ifndef BTY_SHARED_STATE_HPP_
#define BTY_SHARED_STATE_HPP_

struct SharedState {
    int hero_id{0};
    int hero_rank{0};
    int difficulty_level{2};
    int contract{17};
    bool siege{false};
    bool magic{false};
};

#endif // BTY_SHARED_STATE_HPP_
