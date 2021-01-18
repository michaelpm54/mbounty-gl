#ifndef BTY_GAME_USE_MAGIC
#define BTY_GAME_USE_MAGIC

#include <functional>
#include <vector>

#include "engine/component.hpp"
#include "engine/dialog.hpp"

namespace bty {
class Engine;
}

class UseMagic : public Component {
public:
    UseMagic(bty::Engine &engine);
    void load() override;
    void enter() override;
    void bindSpell(int id, std::function<void()> callback);
    bool isOverlay() const override;
    bool handleEvent(Event event) override;
    bool handleKey(Key key) override;

private:
    bty::Engine &_engine;
    bty::Dialog _dlg;
    bool _combat;
    std::unordered_map<int, std::function<void()>> _callbacks;
};

#endif    // BTY_GAME_USE_MAGIC
