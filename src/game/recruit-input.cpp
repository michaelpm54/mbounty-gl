#include "game/recruit-input.hpp"

void RecruitInput::setMax(int max)
{
    _maxAmount = max;

    if (_currentAmount > _maxAmount) {
        _currentAmount = _maxAmount;
    }
}

void RecruitInput::handleKeyDown(Key key)
{
    _anyKeyDown = true;
    switch (key) {
        case Key::Down:
            [[fallthrough]];
        case Key::Left:
            _isDecreasing = true;
            _currentIncrement = -1;
            _timer = 0.099f;
            break;
        case Key::Up:
            [[fallthrough]];
        case Key::Right:
            _isIncreasing = true;
            _currentIncrement = 1;
            _timer = 0.099f;
            break;
        default:
            break;
    }
}

void RecruitInput::handleKeyUp(Key key)
{
    _anyKeyDown = false;
    switch (key) {
        case Key::Right:
            [[fallthrough]];
        case Key::Up:
            _isIncreasing = false;
            _currentIncrement = 0;
            _amountAddedWhileHolding = 0;
            _timer = 0;
            break;
        case Key::Left:
            [[fallthrough]];
        case Key::Down:
            _isDecreasing = false;
            _currentIncrement = 0;
            _amountAddedWhileHolding = 0;
            _timer = 0;
            break;
        default:
            break;
    }
}

bool RecruitInput::update(float dt)
{
    if (!_anyKeyDown) {
        return false;
    }

    if (_isIncreasing) {
        _timer += dt;
        if (_amountAddedWhileHolding == 10) {
            _currentIncrement = 10;
        }
    }
    else if (_isDecreasing) {
        _timer += dt;
        if (_amountAddedWhileHolding == -10) {
            _currentIncrement = -10;
        }
    }

    if (_timer > 0.1f) {
        _currentAmount += _currentIncrement;
        _amountAddedWhileHolding += _currentIncrement;
        _timer = 0;
        if (_currentAmount < 0) {
            _currentAmount = 0;
        }
        if (_currentAmount > _maxAmount) {
            _currentAmount = _maxAmount;
        }
        _currentAmount = _maxAmount < _currentAmount ? _maxAmount : _currentAmount;
        return true;
    }

    return false;
}

int RecruitInput::getCurrentAmount() const
{
    return _currentAmount;
}

void RecruitInput::clear()
{
    _maxAmount = 0;
    _currentAmount = 0;
    _timer = 0;
    _amountAddedWhileHolding = 0;
    _isIncreasing = false;
    _isDecreasing = false;
    _currentIncrement = 0;
}
