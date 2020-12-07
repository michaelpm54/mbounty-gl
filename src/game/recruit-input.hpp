#ifndef BTY_RECRUIT_INPUT_HPP_
#define BTY_RECRUIT_INPUT_HPP_

class RecruitInput {
public:
    void set_max(int max);
    void key(int key, int action);
    void update(float dt);
    int get_current() const;
    void clear();

private:
    int max_ {0};
    int cur_ {0};
    int increment_ {1};
    int amount_added_while_holding_ {0};
    float timer_ {0};
    bool decreasing_ {false};
    bool increasing_ {false};
};

#endif    // BTY_RECRUIT_INPUT_HPP_
