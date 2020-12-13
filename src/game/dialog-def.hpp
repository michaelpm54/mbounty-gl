#ifndef BTY_GAME_DIALOG_DEF_HPP_
#define BTY_GAME_DIALOG_DEF_HPP_

#include <functional>
#include <string>
#include <vector>

namespace bty {
class Dialog;
}

struct DialogDef {
    struct StringDef {
        int x;
        int y;
        std::string str;
    };
    struct Callbacks {
        std::function<void(int)> confirm {nullptr};
        std::function<void()> back {nullptr};
        std::function<void(bty::Dialog &dialog)> up {nullptr};
        std::function<void(bty::Dialog &dialog)> down {nullptr};
        std::function<void(bty::Dialog &dialog)> left {nullptr};
        std::function<void(bty::Dialog &dialog)> right {nullptr};
    };
    int x;
    int y;
    int w;
    int h;
    std::vector<StringDef> strings;
    std::vector<StringDef> options;
    std::vector<bool> visible_options;
    Callbacks callbacks {};
    bool pop_on_confirm {true};
    bool steals_input {true};
};

#endif    // BTY_GAME_DIALOG_DEF_HPP_
