#ifndef BTY_ENGINE_ENGINE_HPP_
#define BTY_ENGINE_ENGINE_HPP_

#include "engine/events.hpp"
#include "engine/gui.hpp"
#include "engine/scene-manager.hpp"
#include "game/game-options.hpp"
#include "gfx/gfx.hpp"
#include "window/window-engine-interface.hpp"

class Battle;
class Ingame;
class Garrison;
class Shop;
class Town;
class SaveManager;
struct ShopInfo;
struct TownGen;

namespace bty {

struct Window;

class Engine {
public:
    Engine(Window &window);
    void run();
    void key(Key key);
    void quit();
    void event(Event event);
    GameOptions &getGameOptions();
    GUI &getGUI();

    void startSiegeBattle(int castleId);
    void startEncounterBattle(int mobId);
    void winSiegeBattle(int castleId);
    void winEncounterBattle(int mobId);
    void loseBattle();

    void acceptWizardOffer();

    void openGarrison(int castleId);
    void openShop(ShopInfo &info);
    void openTown(TownGen *info);

    void setBoatPosition(float x, float y);

    void loadState(const std::string &filename);
    void saveState(const std::string &filename);

    void openSaveManager(bool toLoad);

private:
    InputHandler _inputLayer;
    Window *_window {nullptr};
    glm::mat4 _view;
    GUI _gui;
    bool _run {true};

    Text _btFPSLabel;
    Text _btFPS;

    GameOptions _gameOptions;

    /* Components */
    Battle *_battle;
    Ingame *_ingame;
    Garrison *_garrison;
    Shop *_shop;
    Town *_town;
    SaveManager *_saveManager;
};

}    // namespace bty

#endif    // BTY_ENGINE_ENGINE_HPP_
