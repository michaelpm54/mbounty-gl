#ifndef BTY_GAME_BATTLE_HPP_
#define BTY_GAME_BATTLE_HPP_

#include <array>

#include "engine/component.hpp"
#include "engine/dialog.hpp"
#include "engine/textbox.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Engine;
}    // namespace bty

struct Unit;

enum ActionId {
    AidMeleeAttack,
    AidShootAttack,
    AidTryMove,
    AidMove,
    AidFly,
    AidWait,
    AidPass,
    AidRetaliate,
    AidTryShoot,
    AidSpellClone,
    AidSpellTeleport,
    AidSpellFreeze,
    AidSpellResurrect,
    AidSpellFireball,
    AidSpellLightning,
    AidSpellTurnUndead,
};

struct Action {
    ActionId id;
    glm::ivec2 from;
    glm::ivec2 to;
    std::string fmtstr;
    bool nextUnit {true};
};

class Battle : public Component {
public:
    Battle(bty::Engine &engine);

    void load() override;
    void enter() override;
    void render() override;
    bool handleEvent(Event event) override;
    bool handleKey(Key key) override;
    void update(float dt) override;

    void startSiegeBattle(int castleId);
    void startEncounterBattle(int mobId);

private:
    void initArmies();
    void initBattle();
    void generateTerrain();
    int addVictoryGold();
    void relocateVillain(int villainId);
    bool battleEnd();
    int checkWinner();

    enum class Cursor {
        Fly,
        Move,
        Melee,
        Shoot,
        Magic,
        None,
    };

    struct UnitState {
        int id;
        int startCount;
        int turnCount;
        int count;
        int hp;
        int injury;
        int ammo;
        int moves;
        int waits;
        int x;
        int y;
        bool outOfControl;
        bool frozen;
        bool flying;
        bool retaliated;
    };

    void boardMoveUnitTo(int team, int unit, int x, int y);
    bool boardAnyEnemyAround() const;
    bool boardAnyEnemyAround(int team, int unit) const;
    void boardClearDeadUnits();
    std::tuple<int, bool> boardGetUnitAt(int x, int y) const;
    bool boardTileBlocked(int x, int y) const;
    bool boardBlocked() const;
    bool boardBlocked(int team, int unit) const;
    glm::ivec2 boardGetAdjacentTile(int playerUnit) const;

    void uiMoveCursorDir(int dir);
    void uiShowHitMarker(int x, int y);
    void uiHideHitMarker();
    void uiSetCursorPosition(int x, int y);
    void uiUpdateCounts();
    void uiConfirm();
    void uiConfirmSpell();
    void uiConfirmMenu(int opt);
    void uiSetStatus(const std::string &msg, bool waitForEnter = false);
    void uiSetCursorMode(Cursor cursor);
    void uiUpdateState();
    void uiUpdateCursor();
    void uiUpdateStatus();
    void uiUpdateCurrentUnit();
    void uiUpdateCount(int team, int unit);

    void pauseShow();
    void pauseViewArmy();
    void pauseUseMagic();
    void pauseGiveUp();

    void battleResetMoves();
    int battleAttack(int fromTeam, int fromUnit, int toTeam, int toUnit, bool shoot, bool magic, bool retaliation, int magicDmg);
    int battleDamage(int fromTeam, int fromUnit, int toTeam, int toUnit, bool isRanged, bool isExternal, int externalDmg, bool retaliation);
    void battleVictory();
    void battleDefeat();
    std::string battleGetName() const;
    int battleGetNextUnit() const;
    void battleSwitchTeam();
    void battleSetMoveState();
    UnitState &battleGetUnit();
    const UnitState &battleGetUnit() const;
    void battleDelayThen(std::function<void()> callback);
    void battleOnMove();
    void battleDoAction(Action action);
    void battleUseSpell(int spell);
    int battleGetRangedUnit(int *team = nullptr) const;
    int battleGetLowestHpUnit(int *team = nullptr) const;
    int battleEnemyTeam() const;
    bool battleNotUser() const;
    bool battleCanShoot() const;

    void afnTryMove(Action action);
    void afnMove(Action action);
    void afnMeleeAttack(Action action);
    void afnShootAttack(Action action);
    void afnMagicAttack(Action action);
    void afnRetaliate(Action action);
    void afnWait(Action action);
    void afnPass(Action action);
    void afnTryShoot(Action action);

    void afnSpellClone(Action action);
    void afnSpellTeleport(Action action);
    void afnSpellFreeze(Action action);
    void afnSpellResurrect(Action action);
    void afnSpellFireball(Action action);
    void afnSpellLightning(Action action);
    void afnSpellTurnUndead(Action action);

    void aiMakeAction();

private:
    bty::Engine &_engine;
    glm::mat4 _camera {1.0f};
    int _castleId {-1};
    int _mobId {-1};
    bty::Dialog _dlgVictoryVsMobs;
    bty::Dialog _dlgVictoryVsVillain;
    bty::Text *_btVictoryVsMobs;
    bty::Text *_btVictoryVsVillain;
    std::array<std::array<bty::Text, 5>, 2> _btCounts;
    bty::Sprite _spBg;
    bty::Sprite _spCursor;
    bty::Sprite _spCurrent;
    std::array<std::array<bty::Sprite, 5>, 2> _spUnits;
    bty::Sprite _spHitMarker;
    std::array<bty::Sprite, 6> _spTerrain;
    std::array<const bty::Texture *, 25> _texUnits;
    std::array<const bty::Texture *, 3> _texObstacles;
    const bty::Texture *_texCursorMove;
    const bty::Texture *_texCursorMelee;
    const bty::Texture *_texCursorShoot;
    const bty::Texture *_texCursorMagic;
    const bty::Texture *_texEncounterBg;
    const bty::Texture *_texSiegeBg;
    const bty::Texture *_texCurrentFriendly;
    const bty::Texture *_texCurrentEnemy;
    const bty::Texture *_texCurrentOOC;
    int _cursorX {0};
    int _cursorY {0};
    glm::ivec2 _curUnit {0, 0};
    std::array<std::array<int, 5>, 2> _armies;
    int _cursorDistX {0};
    int _cursorDistY {0};
    std::array<std::array<UnitState, 5>, 2> _unitStates;
    int _usingSpellID {-1};
    bool _usedSpellThisTurn {false};
    bool _choosingTeleportDest {false};
    int _teleportTargetUnit {-1};
    int _teleportTargetTeam {-1};
    float _delayTimer {0};
    float _realDelayDuration {1.2f};
    bool _siege {false};
    std::array<int, 5> *_extEnemyArmy;
    std::array<int, 5> *_extEnemyCounts;
    std::array<int, 30> _terrain;
    std::function<void()> _delayCallback {nullptr};
    bool _hitMarkerVisible {false};
    Cursor _cursorMode {Cursor::Move};
    bool _inDelay {false};
    bool _cursorConstrained {false};
    std::array<bty::Rect, 8> _costSquares;
    bty::Font _boardFont;
};

#endif    // BTY_GAME_BATTLE_HPP_
