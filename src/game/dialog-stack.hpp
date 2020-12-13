#ifndef BTY_GAME_DIALOG_STACK_HPP_
#define BTY_GAME_DIALOG_STACK_HPP_

#include <memory>
#include <vector>

#include "game/dialog-def.hpp"
#include "gfx/dialog.hpp"

class DialogStack {
public:
    DialogStack(bty::Assets &assets);
    bty::Dialog *show_dialog(const DialogDef &dialog, bty::BoxColor color = bty::BoxColor::None);
    bool key(int key, int action); /* Returns whether or not it consumed the key. */
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void update(float dt);
    bool empty() const;
    void set_default_color(bty::BoxColor color);
    void pop();

private:
    bty::Assets &assets_;
    std::vector<std::pair<std::shared_ptr<bty::Dialog>, DialogDef>> stack_;
    bty::BoxColor default_color_ {bty::BoxColor::Intro};
};

#endif    // BTY_GAME_DIALOG_STACK_HPP_
