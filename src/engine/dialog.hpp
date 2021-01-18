#ifndef BTY_ENGINE_DIALOG_HPP_
#define BTY_ENGINE_DIALOG_HPP_

#include <array>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <unordered_set>
#include <vector>

#include "data/color.hpp"
#include "engine/events.hpp"
#include "engine/textbox.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/texture.hpp"
#include "window/keys.hpp"

namespace bty {

class Text;
struct Texture;

class Option : public Text {
public:
    void enable();
    void disable();
    bool enabled() const;

    void show();
    void hide();
    bool visible() const;

    void setEnabled(bool);
    void setVisible(bool);

private:
    bool _enabled {true};
    bool _visible {true};
};

class Dialog : public TextBox {
public:
    /* clang-format off */
    void create(
	int x, int y,
	int w, int h,
	bool setCommonBindings = true
    );
    /* clang-format on */
    void setCellPosition(int x, int y);
    Option *addOption(int x, int y, std::string str = "");
    void next();
    void prev();
    void setSelection(int index);
    void update(float dt);
    int getSelection() const;
    void clearOptions();
    std::deque<Option> &get_options();
    bool handleEvent(Event event);
    void render();
    void bind(Key key, std::function<void(int)> callback);
    void disableInput();
    void enableInput();
    bool inputEnabled() const;
    void onKey(std::function<bool(Key)> callback);

private:
    void updateArrow();

private:
    std::vector<glm::ivec2> _optCellPositions;
    std::deque<Option> _options;
    Sprite _spArrow;
    int _selection {0};
    bool _arrowVisible {true};
    bool _inputEnabled {true};
    std::unordered_map<Key, std::function<void(int)>> _keyBindings;
    std::function<bool(Key)> _keyCallback {nullptr};
};

}    // namespace bty

#endif    // BTY_ENGINE_DIALOG_HPP_
