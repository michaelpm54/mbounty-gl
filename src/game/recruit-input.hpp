#ifndef BTY_RECRUIT_INPUT_HPP_
#define BTY_RECRUIT_INPUT_HPP_

#include "window/keys.hpp"

class RecruitInput {
public:
    void setMax(int max);
    void handleKeyDown(Key key);
    void handleKeyUp(Key key);
    bool update(float dt);
    int getCurrentAmount() const;
    void clear();

private:
    int _maxAmount {0};
    int _currentAmount {0};
    int _currentIncrement {1};
    int _amountAddedWhileHolding {0};
    float _timer {0};
    bool _isDecreasing {false};
    bool _isIncreasing {false};
    bool _anyKeyDown {false};
};

#endif    // BTY_RECRUIT_INPUT_HPP_
