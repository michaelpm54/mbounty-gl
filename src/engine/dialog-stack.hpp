#ifndef BTY_ENGINE_DIALOG_STACK_HPP_
#define BTY_ENGINE_DIALOG_STACK_HPP_

#include <memory>
#include <vector>

#include "data/color.hpp"
#include "engine/dialog-def.hpp"
#include "engine/dialog.hpp"

namespace bty {

class DialogStack {
public:
    DialogStack(Assets &assets);
    Dialog *show_dialog(const DialogDef &dialog, BoxColor color = BoxColor::None);
    bool key(int key, int action); /* Returns whether or not it consumed the key. */
    void draw(Gfx &gfx, glm::mat4 &camera);
    void update(float dt);
    bool empty() const;
    void set_default_color(BoxColor color);
    void pop();

private:
    Assets &assets_;
    std::vector<std::pair<std::shared_ptr<Dialog>, DialogDef>> stack_;
    BoxColor default_color_ {BoxColor::Intro};
};

}    // namespace bty

#endif    // BTY_ENGINE_DIALOG_STACK_HPP_
