#ifndef BTY_GUI_HPP
#define BTY_GUI_HPP

#include "engine/component.hpp"
#include "engine/dialog.hpp"
#include "game/hud.hpp"

namespace bty {

class GUI {
public:
    GUI();
    void pushDialog(Dialog &dialog);
    void popDialog();
    bool handleEvent(Event event);
    void render();
    void update(float dt);
    void showHUD();
    void hideHUD();
    Hud &getHUD();
    bool hasDialog() const;
    void showMessage(int x, int y, int w, int h, const std::string &message);
    std::shared_ptr<Dialog> makeDialog(int x, int y, int w, int h, bool backspacePops = true);

private:
    Hud _hud;
    bool _hudVisible {false};
    std::vector<Dialog *> _dialogs;
    std::vector<std::shared_ptr<Dialog>> _madeDialogs;
    glm::mat4 _view {1.0f};
};

}    // namespace bty

#endif    // BTY_GUI_HPP
