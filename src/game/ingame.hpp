#ifndef BTY_GAME_INGAME_HPP_
#define BTY_GAME_INGAME_HPP_

#include "engine/component.hpp"
#include "engine/dialog.hpp"
#include "engine/engine.hpp"
#include "engine/textbox.hpp"
#include "engine/timer.hpp"
#include "game/battle.hpp"
#include "game/chest-commission.hpp"
#include "game/chest-generator.hpp"
#include "game/chest-gold.hpp"
#include "game/chest-spell-capacity.hpp"
#include "game/chest-spell-power.hpp"
#include "game/chest-spell.hpp"
#include "game/defeat.hpp"
#include "game/dir-flags.hpp"
#include "game/game-controls.hpp"
#include "game/garrison.hpp"
#include "game/hero.hpp"
#include "game/kings-castle.hpp"
#include "game/map.hpp"
#include "game/mob.hpp"
#include "game/save.hpp"
#include "game/shop-info.hpp"
#include "game/shop.hpp"
#include "game/town.hpp"
#include "game/use-magic.hpp"
#include "game/victory.hpp"
#include "game/view-army.hpp"
#include "game/view-character.hpp"
#include "game/view-continent.hpp"
#include "game/view-contract.hpp"
#include "game/view-puzzle.hpp"
#include "game/wizard.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Gfx;
}    // namespace bty

class Hud;
struct GLFWwindow;
struct c2AABB;
class Game;

class Ingame : public Component {
public:
    Ingame(bty::Engine &engine);

    bool handleEvent(Event event) override;
    bool handleKeyDown(Key key);
    bool handleKeyUp(Key key);
    void update(float dt) override;
    void render() override;
    void renderLate() override;
    void load() override;
    void enter() override;

    void setup();
    void updateAnimations(float dt);

    void winSiegeBattle(int castleId);
    void winEncounterBattle(int mobId);
    void acceptWizardOffer();
    void setBoatPosition(float x, float y);
    void disgrace();

    void saveState(std::ofstream &f);
    void loadState(std::ifstream &f);

private:
    void defeat();
    void victory();
    void genTiles();
    void updateMobs(float dt);
    void drawMobs();
    bool addUnitToArmy(int id, int count);
    void spellBridge();
    void spellTimestop();
    void spellFindVillain();
    void spellInstantArmy();
    void spellRaiseControl();
    void spellTCGate(bool town);
    void confirmTownGate(int opt);
    void confirmCastleGate(int opt);
    void placeBridgeAt(int x, int y, int continent, bool horizontal);
    void dismiss();
    void useMagic();
    void dismissSlot(int slot);
    void endWeekBudget(bool search);
    void endWeekAstrology(bool search);

    /* Timers */
    void dayTick();
    void timestopTick();
    void automoveTick();

    /* Movement */
    bool moveIncrement(c2AABB &box, float dx, float dy, Tile &centerTile, Tile &collidedTile, bool (Ingame::*canMove)(int), bool mob);
    void updateCamera();
    void sailTo(int continent);
    void updateVisitedTiles();
    void moveHero(float dt);
    void moveHeroTo(int x, int y, int c);
    void moveMob(Mob &entity, float dt, const glm::vec2 &dir);
    void flyLand();
    void automove(float dt);

    /* Collision */
    bool heroCanMove(int id);
    bool mobCanMove(int id);
    bool events(const Tile &tile, bool &teleport);
    void collideSign(const Tile &tile);
    void collideTown(const Tile &tile);
    void collideShop(const Tile &tile);
    void collideChest(const Tile &tile);
    void collideCastle(const Tile &tile);
    void collideArtifact(const Tile &tile);
    void collideTeleportCave(const Tile &tile);

    void doSearch();
    void failSearch();

    void endWeek(bool search);

    std::vector<Mob *> getMobsInRange(int x, int y, int range);

    void handlePauseOptions(int opt);
    void pause();

    void moveCameraToSceptre();
    void tryJoin(int id, int count, std::function<void()> onOption);
    void bridgeDir(DirFlags dir);

private:
    bty::Engine &_engine;

    UseMagic *_useMagic;

    bty::Dialog _dlgPause;
    bty::Dialog _dlgJoin;
    bty::Dialog _dlgSailMap;
    bty::Dialog _dlgContMap;
    bty::Dialog _dlgArtifact;
    bty::Dialog _dlgSearch;
    bty::Dialog _dlgSearchFail;
    bty::Dialog _dlgBridge;
    bty::Dialog _dlgTCGate;

    bty::Text *_btSignText;
    bty::Text *_btFleeDescriptor;
    bty::Text *_btJoinDescriptor;
    bty::Text *_btSailMapDest;
    bty::Text *_btArtifactMessage;
    bty::Text *_btTCGate0;
    bty::Text *_btTCGate1;

    int _moveFlags {DIR_FLAG_NONE};
    bool _loaded {false};
    bool _paused {false};

    Map _map;
    Hero _spHero;
    glm::mat4 _uiView;
    glm::mat4 _mapView;
    std::array<const bty::Texture *, 25> _texUnits;

    /* Clocks */
    bty::Timer _dayTimer;
    bty::Timer _timestopTimer;
    bty::Timer _automoveTimer;

    int _sceptreContinent {-1};
    int _sceptreX {-1};
    int _sceptreY {-1};

    int _tempPuzzleContinent {-1};

    bty::Sprite _spBoat;

    int _lastWaterX {-1};
    int _lastWaterY {-1};

    bty::Rect _dbgCollisionRect;
    bty::Text _dbgTileText;
    Tile _dbgLastTile {-1, -1, -1};
    Tile _dbgLastEventTile {-1, -1, -1};

    ChestGenerator _chestGenerator;
    ChestGold _chestGold;
    ChestCommission _chestCommission;
    ChestSpellPower _chestSpellPower;
    ChestSpellCapacity _chestSpellCapacity;
    ChestSpell _chestSpell;
};

#endif    // BTY_GAME_INGAME_HPP_
