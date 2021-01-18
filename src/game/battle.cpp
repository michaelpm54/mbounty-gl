#include "game/battle.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

#include "data/bounty.hpp"
#include "data/castles.hpp"
#include "data/hero.hpp"
#include "data/spells.hpp"
#include "data/villains.hpp"
#include "engine/engine.hpp"
#include "engine/scene-manager.hpp"
#include "engine/texture-cache.hpp"
#include "game/army-gen.hpp"
#include "game/game-controls.hpp"
#include "game/game-options.hpp"
#include "game/hud.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"

static constexpr char const *kSiegeVictoryMessage = {
    R"raw(          Victory!
          ________
   Well done {},
    you have successfully
 vanquished yet another foe.

   Spoils of War: {} gold.
     And the capture of
	  {}!

For fulfilling your contract
  you receive an additional
     {} gold as bounty
  and a piece of the map to
     the stolen sceptre.)raw",
};

static constexpr char const *kSiegeVictoryMessageNoContract = {
    R"raw(          Victory!
          ________
   Well done {},
    you have successfully
 vanquished yet another foe.

   Spoils of War: {} gold.
     And the capture of
	  {}!


   Since you did not have
    the proper contract,
 the Lord has been set free.)raw",
};

static constexpr char const *kEncounterVictoryMessage = {
    R"raw(          Victory!
          ________
   Well done {},
    you have successfully
 vanquished yet another foe.

   Spoils of War: {} gold.
	)raw",
};

Battle::Battle(bty::Engine &engine)
    : _engine(engine)
    , _camera(glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f))
{
}

void Battle::load()
{
    auto &textures {Textures::instance()};
    auto &font = textures.getFont();

    _texEncounterBg = textures.get("battle/encounter.png");
    _texSiegeBg = textures.get("battle/siege.png");

    _spBg.setPosition(8, 24);
    _texCurrentFriendly = textures.get("battle/active-unit.png", {5, 2});
    _texCurrentEnemy = textures.get("battle/enemy.png", {10, 1});
    _texCurrentOOC = textures.get("battle/out-of-control.png", {10, 1});
    _spCurrent.setTexture(_texCurrentFriendly);
    _spHitMarker.setTexture(textures.get("battle/damage-marker.png", {4, 1}));
    _spHitMarker.setAnimationRepeat(false);

    _texCursorMove = textures.get("battle/selection.png", {4, 1});
    _texCursorMelee = textures.get("battle/melee.png", {4, 1});
    _texCursorShoot = textures.get("battle/shoot.png", {4, 1});
    _texCursorMagic = textures.get("battle/magic.png", {4, 1});

    for (int i = 0; i < UnitId::UnitCount; i++) {
        _texUnits[i] = textures.get(fmt::format("units/{}.png", i), {2, 2});
    }

    _texObstacles[0] = textures.get("battle/obstacle-0.png");
    _texObstacles[1] = textures.get("battle/obstacle-1.png");
    _texObstacles[2] = textures.get("battle/obstacle-2.png", {10, 1});

    _boardFont.loadFromTexture(textures.get("fonts/board-font.png"), {8, 8});

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            _btCounts[i][j].setFont(_boardFont);
        }
    }

    for (int i = 0; i < 8; i++) {
        _costSquares[i].setSize({48.0f, 40.0f});
    }

    _dlgVictoryVsMobs.create(5, 10, 30, 9);
    _btVictoryVsMobs = _dlgVictoryVsMobs.addString(1, 1);
    _dlgVictoryVsMobs.bind(Key::Enter, [this](int) {
        _engine.getGUI().popDialog();
        _engine.winEncounterBattle(_mobId);
    });
    _dlgVictoryVsMobs.bind(Key::Backspace, [this](int) {
        _engine.getGUI().popDialog();
        _engine.winEncounterBattle(_mobId);
    });

    _dlgVictoryVsVillain.create(5, 6, 30, 18);
    _btVictoryVsVillain = _dlgVictoryVsVillain.addString(1, 1);
    _dlgVictoryVsVillain.bind(Key::Enter, [this](int) {
        _engine.getGUI().popDialog();
        _engine.winSiegeBattle(_castleId);
    });
    _dlgVictoryVsVillain.bind(Key::Backspace, [this](int) {
        _engine.getGUI().popDialog();
        _engine.winSiegeBattle(_castleId);
    });
}

void Battle::enter()
{
    State::combat = true;
    _castleId = -1;
    _mobId = -1;

    _realDelayDuration = _engine.getGameOptions().combat_delay * 0.24f;

    _engine.getGUI().getHUD().setBlankFrame();

    _delayTimer = 0;

    auto color {bty::getBoxColor(State::difficulty)};
    _dlgVictoryVsMobs.setColor(color);
    _dlgVictoryVsVillain.setColor(color);
}

void Battle::render()
{
    GFX::instance().drawSprite(_spBg);

    for (int i = 0; i < 6; i++) {
        GFX::instance().drawSprite(_spTerrain[i]);
    }

    if (_engine.getGameOptions().debug) {
        if (_curUnit.x == 1 && !battleGetUnit().flying) {
            for (int i = 0; i < 8; i++) {
                GFX::instance().drawRect(_costSquares[i]);
            }
        }
    }

    auto &gfx {GFX::instance()};

    gfx.drawSprite(_spCurrent);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (_armies[i][j] == -1) {
                continue;
            }
            gfx.drawSprite(_spUnits[i][j]);
        }
    }

    gfx.drawSprite(_spCursor);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (_armies[i][j] == -1) {
                continue;
            }
            gfx.drawText(_btCounts[i][j]);
        }
    }

    if (_hitMarkerVisible) {
        gfx.drawSprite(_spHitMarker);
    }
}

bool Battle::handleEvent(Event event)
{
    if (event.id == EventId::KeyDown) {
        return handleKey(event.key);
    }
    return false;
}

bool Battle::handleKey(Key key)
{
    switch (key) {
        case Key::Left:
            if (_curUnit.x == 0) {
                uiMoveCursorDir(0);
            }
            break;
        case Key::Right:
            if (_curUnit.x == 0) {
                uiMoveCursorDir(1);
            }
            break;
        case Key::Up:
            if (_curUnit.x == 0) {
                uiMoveCursorDir(2);
            }
            break;
        case Key::Down:
            if (_curUnit.x == 0) {
                uiMoveCursorDir(3);
            }
            break;
        case Key::Enter:
            if (_curUnit.x == 0) {
                uiConfirm();
            }
            break;
        case Key::Space:
            pauseShow();
            break;
        case Key::V:
            battleVictory();
            break;
        default:
            return false;
    }
    return true;
}

void Battle::update(float dt)
{
    if (_hitMarkerVisible) {
        _spHitMarker.update(dt);
        if (_spHitMarker.isAnimationDone()) {
            uiHideHitMarker();
        }
    }

    if (_inDelay) {
        _delayTimer += dt;
        if (_delayTimer >= _realDelayDuration) {
            _inDelay = false;
            _delayTimer = 0;
            if (_delayCallback) {
                auto cb = _delayCallback;
                _delayCallback = nullptr;
                cb();

                if (battleEnd()) {
                    return;
                }
            }
        }
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (_armies[i][j] == -1) {
                continue;
            }
            _spUnits[i][j].update(dt);
        }
    }

    _spCursor.update(dt);
    _spCurrent.update(dt);

    for (auto &sprite : _spTerrain) {
        sprite.update(dt);
    }

    if (_curUnit.x == 1 || _curUnit.x == 0 && battleGetUnit().outOfControl) {
        if (_inDelay) {
            return;
        }

        if (battleEnd()) {
            return;
        }

        aiMakeAction();
    }
}

std::string Battle::battleGetName() const
{
    if (_curUnit.x == -1 || _curUnit.y == -1) {
        return "INVALID";
    }
    return kUnits[_armies[_curUnit.x][_curUnit.y]].namePlural;
}

void Battle::uiUpdateState()
{
    if (battleEnd()) {
        return;
    }

    uiUpdateStatus();
    uiUpdateCursor();
    uiUpdateCurrentUnit();
}

void Battle::uiUpdateCursor()
{
    auto &unit = battleGetUnit();

    if (battleNotUser()) {
        uiSetCursorMode(Cursor::None);
    }
    else if (unit.flying) {
        uiSetCursorMode(Cursor::Fly);
    }
    else {
        uiSetCursorMode(Cursor::Move);
    }
}

void Battle::uiUpdateStatus()
{
    if (battleNotUser()) {
        return;
    }

    auto &unit = battleGetUnit();

    if (unit.flying) {
        uiSetStatus(fmt::format("{} fly", battleGetName()));
    }
    else if (battleCanShoot()) {
        uiSetStatus(fmt::format("{} Attack, Shoot or Move {}", battleGetName(), unit.moves));
    }
    else {
        uiSetStatus(fmt::format("{} Attack or Move {}", battleGetName(), unit.moves));
    }
}

void Battle::uiUpdateCurrentUnit()
{
    auto &unit = battleGetUnit();

    _spCurrent.setPosition(16.0f + unit.x * 48.0f, 24.0f + unit.y * 40.0f);
    uiSetCursorPosition(unit.x, unit.y);

    unit.outOfControl = _curUnit.x == 0 && (unit.hp * unit.count) > State::leadership;

    if (unit.outOfControl) {
        _spCurrent.setTexture(_texCurrentOOC);
    }
    else if (_curUnit.x == 1) {
        _spCurrent.setTexture(_texCurrentEnemy);
    }
    else {
        _spCurrent.setTexture(_texCurrentFriendly);
    }
}

void Battle::boardMoveUnitTo(int team, int unit, int x, int y)
{
    if (team > 1) {
        spdlog::warn("Battle::boardMoveUnitTo: Team {} is invalid", team);
        return;
    }
    if (unit < 0 || unit > 4) {
        spdlog::warn("Battle::boardMoveUnitTo: Unit {} is invalid", unit);
        return;
    }

    float x_ = 16.0f + x * 48.0f;
    float y_ = 24.0f + y * 40.0f;
    _spUnits[team][unit].setPosition(x_, y_);
    _unitStates[team][unit].x = x;
    _unitStates[team][unit].y = y;
    uiUpdateCount(team, unit);
    _cursorDistX = 0;
    _cursorDistY = 0;
}

void Battle::uiMoveCursorDir(int dir)
{
    const auto &unit {battleGetUnit()};

    switch (dir) {
        case 0:    // left
            if (_cursorX == 0) {
                return;
            }
            if (!_cursorConstrained) {
                _cursorX--;
            }
            else if (_cursorDistX > -1) {
                _cursorX--;
                _cursorDistX--;
            }
            break;
        case 1:    // right
            if (_cursorX == 5) {
                return;
            }
            if (!_cursorConstrained) {
                _cursorX++;
            }
            else if (_cursorDistX < 1) {
                _cursorX++;
                _cursorDistX++;
            }
            break;
        case 2:    // up
            if (_cursorY == 0) {
                return;
            }
            if (!_cursorConstrained) {
                _cursorY--;
            }
            else if (_cursorDistY > -1) {
                _cursorY--;
                _cursorDistY--;
            }
            break;
        case 3:    // down
            if (_cursorY == 4) {
                return;
            }
            if (!_cursorConstrained) {
                _cursorY++;
            }
            else if (_cursorDistY < 1) {
                _cursorY++;
                _cursorDistY++;
            }
            break;
        default:
            break;
    }

    uiSetCursorPosition(_cursorX, _cursorY);

    if (_cursorConstrained) {
        auto [unit, enemy] = boardGetUnitAt(_cursorX, _cursorY);
        (void)unit;
        if (enemy) {
            uiSetCursorMode(Cursor::Melee);
        }
        else {
            uiSetCursorMode(Cursor::Move);
        }
    }
}

void Battle::uiConfirm()
{
    switch (_cursorMode) {
        case Cursor::Melee:
            [[fallthrough]];
        case Cursor::Move:
            [[fallthrough]];
        case Cursor::Fly:
            battleDoAction({AidTryMove, _curUnit, {_cursorX, _cursorY}});
            break;
        case Cursor::Shoot:
            battleDoAction({AidTryShoot, _curUnit, {_cursorX, _cursorY}});
            break;
        case Cursor::Magic:
            uiConfirmSpell();
            break;
        default:
            break;
    }
}

void Battle::battleResetMoves()
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (_armies[i][j] == -1) {
                continue;
            }

            const auto &unit = kUnits[_armies[i][j]];
            auto &us = _unitStates[i][j];
            us.turnCount = us.count;
            us.hp = unit.hp;
            us.injury = 0;
            us.outOfControl = (us.hp * us.count) > State::leadership;
            us.flying = (unit.abilities & AbilityFly) && !boardAnyEnemyAround();
            us.moves = unit.initialMoves;
            us.waits = 0;
            us.retaliated = false;
        }
    }

    _usedSpellThisTurn = false;
}

void Battle::uiShowHitMarker(int x, int y)
{
    float x_ = 16.0f + x * 48.0f;
    float y_ = 24.0f + y * 40.0f;

    _spHitMarker.resetAnimation();
    _spHitMarker.setPosition(x_, y_);

    _hitMarkerVisible = true;
}

void Battle::uiHideHitMarker()
{
    _hitMarkerVisible = false;
}

int Battle::battleAttack(int fromTeam, int fromUnit, int toTeam, int toUnit, bool shoot, bool magic, bool retaliation, int magicDmg)
{
    uiShowHitMarker(_unitStates[toTeam][toUnit].x, _unitStates[toTeam][toUnit].y);
    return battleDamage(fromTeam, fromUnit, toTeam, toUnit, shoot, magic, magicDmg, retaliation);
}

std::tuple<int, bool> Battle::boardGetUnitAt(int x, int y) const
{
    const glm::ivec2 pos {x, y};

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (_armies[i][j] == -1) {
                continue;
            }
            if (pos == glm::ivec2 {_unitStates[i][j].x, _unitStates[i][j].y}) {
                if (i != _curUnit.x) {
                    return {j, true};
                }
                return {j, false};
            }
        }
    }

    return {-1, false};
}

int unitsKilled(int dmg, int hp)
{
    return dmg / hp;
}

int dmgRemainder(int dmg, int hp)
{
    return dmg % hp;
}

float moraleModifier(int morale)
{
    return morale == 0 ? 1.0f : morale == 1 ? 1.5f
                                            : 0.5f;
}

int Battle::battleDamage(int fromTeam, int fromUnit, int toTeam, int toUnit, bool isRanged, bool isExternal, int externalDmg, bool retaliation)
{
    bool hasSword = State::artifacts_found[ArtiSwordOfProwess];
    bool hasShield = State::artifacts_found[ArtiShieldOfProtection];

    const int unitIdA = _armies[fromTeam][fromUnit];
    const int unitIdB = _armies[toTeam][toUnit];

    const auto &unitA {kUnits[unitIdA]};
    const auto &unitB {kUnits[unitIdB]};

    auto &unitStateA = _unitStates[fromTeam][fromUnit];
    auto &unitStateB = _unitStates[toTeam][toUnit];

    if (!retaliation) {
        unitStateA.turnCount = unitStateA.count;
        unitStateB.turnCount = unitStateB.count;
    }

    bool cancelAttack = false;

    int scytheKills = 0;
    int finalDmg = 0;
    int dmg = 0;

    if (isExternal) {
        //magic-vs-unit
        finalDmg = externalDmg;
    }
    else {
        //unit-vs-unit
        if (unitA.abilities & AbilityScythe) {
            if (((rand() % 100) + 1) > 89) {    // 10% chance
                                                // ceil
                scytheKills = (unitStateB.count + 2 - 1) / 2;
            }
        }

        if (isRanged) {
            --unitStateA.ammo;
            if (unitIdA == UnitId::Druids) {
                if (unitB.abilities & AbilityImmune)
                    cancelAttack = true;
                else
                    dmg = 10;
            }
            if (unitIdA == UnitId::Archmages) {
                if (unitB.abilities & AbilityImmune)
                    cancelAttack = true;
                else
                    dmg = 25;
            }
            else
                dmg = (rand() % unitA.shootMaxDmg) + unitA.shootMinDmg;
        }
        else
            dmg = (rand() % unitA.meleeMaxDmg) + unitA.meleeMinDmg;

        int total = dmg * unitStateA.turnCount;
        int skillDiff = unitA.skillLevel + 5 - unitB.skillLevel;
        finalDmg = (total * skillDiff) / 10;

        if (fromTeam == 0) {
            if (!unitStateA.outOfControl) {
                finalDmg = static_cast<int>(moraleModifier(State::morales[fromUnit]) * static_cast<float>(finalDmg));
            }
        }

        if (fromTeam == 0 && hasSword)
            finalDmg = static_cast<int>(1.5f * static_cast<float>(finalDmg));

        if (toTeam == 0 && hasShield) {
            finalDmg /= 4;
            finalDmg *= 3;
            //almost same as multiplying 0.75, but more brutal, as div by 4 can yield 0
        }
    }

    finalDmg += unitStateB.injury;
    finalDmg += unitStateB.hp * scytheKills;

    int kills = unitsKilled(finalDmg, unitStateB.hp);
    int injury = dmgRemainder(static_cast<int>(finalDmg), unitStateB.hp);

    unitStateB.injury = injury;

    if (kills < unitStateB.count) {
        //stack survives
        unitStateB.count -= kills;
    }
    else {
        //stack dies
        unitStateB.count = 0;
        unitStateB.x = -1;
        unitStateB.y = -1;
        finalDmg = unitStateB.turnCount * unitStateB.hp;
    }

    int final_kills = std::min(kills, unitStateB.turnCount);

    if (fromTeam == 1) {
        State::followers_killed += final_kills;
    }

    /* Leech and absorb */
    if (!isExternal) {
        /* Difference between leech and absorb is, leech can only get back to the original
			count. Absorb has no limit. */
        if (unitA.abilities & AbilityAbsorb)
            unitStateA.count += final_kills;

        else if (unitA.abilities & AbilityLeech) {
            unitStateA.count += final_kills;
            if (unitStateA.count > unitStateA.startCount) {
                unitStateA.count = unitStateA.startCount;
                unitStateA.injury = 0;
            }
        }
    }

    return final_kills;
}

/* It's convenient to do this after any status messages are
displayed which rely on the unit ID. */
void Battle::boardClearDeadUnits()
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (_armies[i][j] == -1) {
                continue;
            }
            if (_unitStates[i][j].count == 0) {
                _armies[i][j] = -1;
            }
        }
    }
    battleEnd();
}

void Battle::uiUpdateCounts()
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (_armies[i][j] == -1 || _unitStates[i][j].count == 0) {
                continue;
            }

            uiUpdateCount(i, j);
        }
    }
}

void Battle::pauseViewArmy()
{
    int army[] = {
        _armies[0][0],
        _armies[0][1],
        _armies[0][2],
        _armies[0][3],
        _armies[0][4],
    };
    int counts[] = {
        _unitStates[0][0].count,
        _unitStates[0][1].count,
        _unitStates[0][2].count,
        _unitStates[0][3].count,
        _unitStates[0][4].count,
    };
    SceneMan::instance().setScene("viewarmy");
}

void Battle::battleDoAction(Action action)
{
    switch (action.id) {
        case AidTryMove:
            afnTryMove(action);
            break;
        case AidMove:
            afnMove(action);
            break;
        case AidMeleeAttack:
            afnMeleeAttack(action);
            break;
        case AidShootAttack:
            afnShootAttack(action);
            break;
        case AidRetaliate:
            afnRetaliate(action);
            break;
        case AidWait:
            afnWait(action);
            break;
        case AidPass:
            afnPass(action);
            break;
        case AidTryShoot:
            afnTryShoot(action);
            break;
        case AidSpellClone:
            afnSpellClone(action);
            break;
        case AidSpellTeleport:
            afnSpellTeleport(action);
            break;
        case AidSpellFreeze:
            afnSpellFreeze(action);
            break;
        case AidSpellResurrect:
            afnSpellResurrect(action);
            break;
        case AidSpellFireball:
            afnSpellFireball(action);
            break;
        case AidSpellLightning:
            afnSpellLightning(action);
            break;
        case AidSpellTurnUndead:
            afnSpellTurnUndead(action);
            break;
        default:
            break;
    }
}

void Battle::afnWait(Action action)
{
    uiSetStatus(fmt::format("{} wait", battleGetName()));
    _unitStates[action.from.x][action.from.y].waits++;
    battleDelayThen([this]() {
        battleOnMove();
    });
}

void Battle::afnPass(Action action)
{
    uiSetStatus(fmt::format("{} pass", battleGetName()));
    _unitStates[action.from.x][action.from.y].moves = 0;
    battleDelayThen([this]() {
        battleOnMove();
    });
}

void Battle::afnTryMove(Action action)
{
    /* Tile was not empty. */
    if (_terrain[action.to.x + action.to.y * 6] != 0) {
        if (_unitStates[action.from.x][action.from.y].flying) {
            uiSetStatus(" You can't land on an occupied area!");
        }
        else {
            uiSetStatus(" You can't move to an occupied area!");
        }
    }
    else {
        auto [target, enemy] = boardGetUnitAt(action.to.x, action.to.y);

        /* Tile contains a unit. */
        if (target != -1) {
            /* Tile was an enemy unit. */
            if (enemy) {
                battleDoAction({AidMeleeAttack, action.from, {enemy ? !_curUnit.x : _curUnit.x, target}});
            }
            /* Targeted self. */
            else if (target == _curUnit.y) {
                /* Can shoot. */
                if (_cursorMode != Cursor::Shoot && battleGetUnit().ammo > 0 && !boardAnyEnemyAround()) {
                    uiSetCursorMode(Cursor::Shoot);
                    uiSetStatus(fmt::format("{} Shoot ({} left)", battleGetName(), battleGetUnit().ammo));
                }
                /* Already shooting, want to wait. */
                else {
                    battleDoAction({AidWait, action.from});
                }
            }
            /* Tile was terrain or friendly unit. */
            else {
                uiSetStatus(" You can't move to an occupied area!");
            }
        }
        else {
            battleDoAction({.id = AidMove, .from = action.from, .to = action.to, .nextUnit = action.nextUnit});
        }
    }
}

void Battle::afnMeleeAttack(Action action)
{
    auto &unit = battleGetUnit();

    bool retaliate = false;

    if (!_unitStates[action.to.x][action.to.y].retaliated) {
        retaliate = true;
        _unitStates[action.to.x][action.to.y].retaliated = true;
    }

    uiSetCursorMode(Cursor::None);

    int kills = battleAttack(action.from.x, action.from.y, action.to.x, action.to.y, false, false, false, 0);

    uiSetStatus(fmt::format("{} attack {}, {} die", battleGetName(), kUnits[_armies[action.to.x][action.to.y]].namePlural, kills));
    battleGetUnit().moves = 0;

    battleDelayThen([this, action, retaliate]() {
        uiUpdateCount(action.to.x, action.to.y);

        if (retaliate) {
            battleDoAction({AidRetaliate, action.to, action.from});
        }
        else {
            boardClearDeadUnits();
            battleOnMove();
        }
    });
}

void Battle::afnShootAttack(Action action)
{
    auto &unit = battleGetUnit();

    uiSetCursorMode(Cursor::None);

    int kills = battleAttack(action.from.x, action.from.y, action.to.x, action.to.y, true, false, false, 0);

    uiSetStatus(fmt::format("{} shoot {}, {} die", battleGetName(), kUnits[_armies[action.to.x][action.to.y]].namePlural, kills));
    battleGetUnit().moves = 0;

    battleDelayThen([this, action]() {
        uiUpdateCount(action.to.x, action.to.y);
        boardClearDeadUnits();
        battleOnMove();
    });
}

void Battle::battleDelayThen(std::function<void()> callback)
{
    _inDelay = true;
    _delayTimer = 0;
    _delayCallback = callback;
}

void Battle::afnRetaliate(Action action)
{
    int kills = battleAttack(action.from.x, action.from.y, action.to.x, action.to.y, false, false, true, 0);
    uiSetStatus(fmt::format("{} retaliate, killing {}", kUnits[_armies[action.from.x][action.from.y]].namePlural, kills));

    battleDelayThen([this, action]() {
        uiUpdateCount(action.to.x, action.to.y);
        boardClearDeadUnits();
        battleOnMove();
    });
}

void Battle::battleSwitchTeam()
{
    _curUnit.x = _curUnit.x == 1 ? 0 : 1;

    int index = 0;

    while (index < 5 && _armies[_curUnit.x][index] == -1) {
        index++;
    }

    _curUnit.y = index;

    if (_curUnit.y == 5) {
        return;
    }

    battleResetMoves();
    if (_unitStates[_curUnit.x][_curUnit.y].frozen && !boardAnyEnemyAround()) {
        uiSetStatus(fmt::format("{} are frozen", battleGetName()));
        battleDelayThen([this]() {
            battleOnMove();
        });
    }
    else {
        uiUpdateState();
    }
}

int Battle::battleGetNextUnit() const
{
    for (int i = 0; i < 5; i++) {
        int idx = (_curUnit.y + i + 1) % 5;
        if (_armies[_curUnit.x][idx] == -1) {
            continue;
        }
        if (_unitStates[_curUnit.x][idx].moves == 0 || _unitStates[_curUnit.x][idx].waits == 2) {
            continue;
        }
        return idx;
    }
    return -1;
}

void Battle::battleOnMove()
{
    int nextUnit = battleGetNextUnit();

    if (nextUnit == -1) {
        battleSwitchTeam();
        _cursorDistX = 0;
        _cursorDistY = 0;
    }
    else {
        _curUnit.y = nextUnit;
        _cursorDistX = 0;
        _cursorDistY = 0;
    }

    if (battleEnd()) {
        return;
    }

    if (!battleNotUser()) {
        uiUpdateState();
    }
}

void Battle::afnMove(Action action)
{
    if (_unitStates[action.from.x][action.from.y].flying) {
        _unitStates[action.from.x][action.from.y].flying = false;
        uiSetCursorMode(Cursor::Move);
    }
    else {
        _unitStates[action.from.x][action.from.y].moves--;
        if (action.nextUnit && _unitStates[action.from.x][action.from.y].moves == 0) {
            battleOnMove();
        }
    }

    _cursorDistX = 0;
    _cursorDistY = 0;

    boardMoveUnitTo(action.from.x, action.from.y, action.to.x, action.to.y);

    uiUpdateState();
}

void Battle::uiConfirmMenu(int opt)
{
    switch (opt) {
        case 0:
            pauseViewArmy();
            break;
        case 1:
            SceneMan::instance().setScene("viewchar");
            break;
        case 2:
            pauseUseMagic();
            break;
        case 3:
            battleDoAction({AidPass, _curUnit});
            break;
        case 4:
            battleDoAction({AidWait, _curUnit});
            break;
        case 5:
            SceneMan::instance().setScene("controls");
            break;
        case 6:
            pauseGiveUp();
            break;
        default:
            break;
    }
}

void Battle::battleUseSpell(int spell)
{
    _usingSpellID = spell - 7;

    uiSetCursorMode(Cursor::Magic);

    switch (_usingSpellID) {
        case 0:    // clone
            uiSetStatus("     Select your army to clone");
            break;
        case 1:    // teleport
            uiSetStatus("       Select army to teleport.");
            break;
        case 2:    // fireball
            uiSetStatus("     Select enemy army to blast.");
            break;
        case 3:    // lightning
            uiSetStatus("  Select enemy army to electricute.");
            break;
        case 4:    // freeze
            uiSetStatus("     Select enemy army to freeze.");
            break;
        case 5:    // resurrect
            uiSetStatus("   Select your army to resurrect.");
            break;
        case 6:    // turn undead
            uiSetStatus("     Select enemy army to turn.");
            break;
        default:
            break;
    }

    State::spells[spell - 7]--;

    _usedSpellThisTurn = true;
}

void Battle::uiConfirmSpell()
{
    auto [target, enemy] = boardGetUnitAt(_cursorX, _cursorY);

    bool is_immune = target == -1 ? false : (_armies[enemy ? 1 : 0][target] == UnitId::Dragons);

    switch (_usingSpellID) {
        case 0:    // clone
            if (target == -1) {
                uiSetStatus("      You must target somebody!");
            }
            else {
                if (enemy) {
                    uiSetStatus("    You must select your own army!");
                }
                else {
                    if (is_immune) {
                        uiSetStatus("Clone has no effect on Dragons");
                    }
                    else {
                        battleDoAction({AidSpellClone, {-1, -1}, {_cursorX, _cursorY}});
                    }
                }
            }
            break;
        case 1:    // teleport
            if (_choosingTeleportDest) {
                if (target != -1) {
                    uiSetStatus("  You must teleport to an empty area!");
                }
                else {
                    battleDoAction({AidSpellTeleport, {_teleportTargetTeam, _teleportTargetUnit}, {_cursorX, _cursorY}});
                }
            }
            else if (target == -1) {
                uiSetStatus("      You must target somebody!");
            }
            else if (is_immune) {
                uiSetStatus("Teleport has no effect on Dragons");
            }
            else {
                uiSetStatus(" Select new location to teleport army.");
                _choosingTeleportDest = true;
                _spCursor.setTexture(_texCursorMove);
                _teleportTargetTeam = enemy ? 1 : 0;
                _teleportTargetUnit = target;
            }
            break;
        case 2:    // fireball
            if (target != -1 && !enemy) {
                uiSetStatus("   You must select an opposing army!");
            }
            else if (target == -1) {
                uiSetStatus("      You must target somebody!");
            }
            else if (is_immune) {
                uiSetStatus("Fireball has no effect on Dragons");
            }
            else {
                battleDoAction({AidSpellFireball, _curUnit, {_cursorX, _cursorY}});
            }
            break;
        case 3:    // lightning
            if (target != -1 && !enemy) {
                uiSetStatus("   You must select an opposing army!");
            }
            else if (target == -1) {
                uiSetStatus("      You must target somebody!");
            }
            else if (is_immune) {
                uiSetStatus("Lightning has no effect on Dragons");
            }
            else {
                battleDoAction({AidSpellLightning, _curUnit, {_cursorX, _cursorY}});
            }
            break;
        case 4:    // freeze
            if (target != -1 && !enemy) {
                uiSetStatus("   You must select an opposing army!");
            }
            else if (target == -1) {
                uiSetStatus("      You must target somebody!");
            }
            else if (is_immune) {
                uiSetStatus("Fireball has no effect on Dragons");
            }
            else {
                battleDoAction({AidSpellFreeze, {-1, -1}, {_cursorX, _cursorY}});
            }
            break;
        case 5:    // resurrect
            if (target != -1 && enemy) {
                uiSetStatus("    You must select your own army!");
            }
            else if (target == -1) {
                uiSetStatus("      You must target somebody!");
            }
            else if (is_immune) {
                uiSetStatus("Resurrect has no effect on Dragons");
            }
            else {
                battleDoAction({AidSpellResurrect, {-1, -1}, {_cursorX, _cursorY}});
            }
            break;
        case 6:    // turn undead
            if (target == -1) {
                uiSetStatus("      You must target somebody!");
            }
            else if (is_immune) {
                uiSetStatus("Turn has no effect on Dragons");
            }
            else if (!enemy) {
                uiSetStatus("   You must select an opposing army!");
            }
            else {
                battleDoAction({AidSpellTurnUndead, _curUnit, {_cursorX, _cursorY}});
            }
            break;
        default:
            break;
    }
}

void Battle::afnSpellTeleport(Action action)
{
    _choosingTeleportDest = false;
    boardMoveUnitTo(action.from.x, action.from.y, action.to.x, action.to.y);
    _spCurrent.setPosition(16.0f + battleGetUnit().x * 48.0f, 24.0f + battleGetUnit().y * 40.0f);
    uiSetStatus(fmt::format("{} are teleported", kUnits[_armies[_teleportTargetTeam][_teleportTargetUnit]].namePlural));
    battleDelayThen([this]() {
        uiUpdateState();
    });
}

void Battle::afnSpellClone(Action action)
{
    int clone_amount = 10 * State::spell_power;
    auto [unit, enemy] = boardGetUnitAt(action.to.x, action.to.y);
    _unitStates[0][unit].count += clone_amount;
    uiSetStatus(fmt::format("{} {} are cloned", clone_amount, kUnits[_armies[0][unit]].namePlural));
    uiUpdateCounts();
    battleDelayThen([this]() {
        uiUpdateState();
    });
}

void Battle::afnSpellFreeze(Action action)
{
    auto [unit, enemy] = boardGetUnitAt(action.to.x, action.to.y);
    int team = enemy ? !_curUnit.x : _curUnit.x;
    _unitStates[team][unit].frozen = true;
    uiSetStatus(fmt::format("{} are frozen", kUnits[_armies[team][unit]].namePlural));
    battleDelayThen([this]() {
        uiUpdateState();
    });
}

void Battle::afnSpellResurrect(Action action)
{
    auto [unit, enemy] = boardGetUnitAt(action.to.x, action.to.y);
    int numResurrected = 20 * State::spell_power;
    auto &us = _unitStates[0][unit];
    numResurrected = std::min(numResurrected, us.startCount - us.count);
    us.count += numResurrected;
    State::followers_killed = std::max(0, State::followers_killed - numResurrected);
    uiSetStatus(fmt::format("{} {} are resurrected", numResurrected, kUnits[_armies[0][unit]].namePlural));
    uiUpdateCounts();
    battleDelayThen([this]() {
        uiUpdateState();
    });
}

void Battle::afnSpellFireball(Action action)
{
    auto &unit = battleGetUnit();

    uiSetCursorMode(Cursor::None);

    int targetTeam = 0;
    int targetUnit = 0;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (_armies[i][j] != -1 && _unitStates[i][j].x == action.to.x && _unitStates[i][j].y == action.to.y) {
                targetTeam = i;
                targetUnit = j;
                break;
            }
        }
    }

    int kills = battleAttack(action.from.x, action.from.y, targetTeam, targetUnit, false, true, false, 25 * State::spell_power);

    uiSetStatus(fmt::format("Fireball kills {} {}", kills, kUnits[_armies[targetTeam][targetUnit]].namePlural));
    battleGetUnit().moves = 0;

    battleDelayThen([=, this]() {
        uiUpdateCount(targetTeam, targetUnit);
        boardClearDeadUnits();
        uiUpdateState();
    });
}

void Battle::afnSpellLightning(Action action)
{
    auto &unit = battleGetUnit();

    uiSetCursorMode(Cursor::None);

    int targetTeam = 0;
    int targetUnit = 0;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (_armies[i][j] != -1 && _unitStates[i][j].x == action.to.x && _unitStates[i][j].y == action.to.y) {
                targetTeam = i;
                targetUnit = j;
                break;
            }
        }
    }

    int kills = battleAttack(action.from.x, action.from.y, targetTeam, targetUnit, false, true, false, 10 * State::spell_power);

    uiSetStatus(fmt::format("Lightning kills {} {}", kills, kUnits[_armies[targetTeam][targetUnit]].namePlural));
    battleGetUnit().moves = 0;

    battleDelayThen([=, this]() {
        uiUpdateCount(targetTeam, targetUnit);
        boardClearDeadUnits();
        uiUpdateState();
    });
}

void Battle::afnSpellTurnUndead(Action action)
{
    auto &unit = battleGetUnit();

    uiSetCursorMode(Cursor::None);

    int targetTeam = -1;
    int targetUnit = -1;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (_armies[i][j] != -1 && _unitStates[i][j].x == action.to.x && _unitStates[i][j].y == action.to.y) {
                targetTeam = i;
                targetUnit = j;
                break;
            }
        }
    }

    if (targetTeam == -1 || targetUnit == -1) {
        spdlog::debug("Failed to find target at {} {}\n", action.to.x, action.to.y);
        return;
    }

    int magicDmg = 0;
    if (!(kUnits[_armies[targetTeam][targetUnit]].abilities & AbilityUndead)) {
        uiSetStatus(fmt::format("{} are not undead!", kUnits[_armies[targetTeam][targetUnit]].namePlural));
        battleDelayThen([=, this]() {
            uiUpdateState();
        });
    }
    else {
        int kills = battleAttack(action.from.x, action.from.y, targetTeam, targetUnit, false, true, false, 50 * State::spell_power);
        uiSetStatus(fmt::format("Turn undead kills {} {}", kills, kUnits[_armies[targetTeam][targetUnit]].namePlural));
        battleGetUnit().moves = 0;
        battleDelayThen([=, this]() {
            uiUpdateCount(targetTeam, targetUnit);
            boardClearDeadUnits();
            uiUpdateState();
        });
    }
}

void Battle::uiSetCursorPosition(int x, int y)
{
    _cursorX = x;
    _cursorY = y;
    _spCursor.setPosition(16.0f + _cursorX * 48.0f, 24.0f + _cursorY * 40.0f);
}

int Battle::checkWinner()
{
    int numAlive[2] = {0, 0};
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (_armies[i][j] != -1) {
                numAlive[i]++;
            }
        }
    }

    if (numAlive[0] == 0) {
        for (int i = 0; i < 5; i++) {
            (*_extEnemyArmy)[i] = _armies[1][i];
            (*_extEnemyCounts)[i] = _unitStates[1][i].count;
        }
        if (_siege) {
            State::siege = false;
        }
        return 1;
    }
    else if (numAlive[1] == 0) {
        for (int i = 0; i < 5; i++) {
            State::army[i] = _armies[0][i];
            State::counts[i] = _unitStates[0][i].count;
        }
        return 0;
    }

    return -1;
}

void Battle::battleVictory()
{
    int goldTotal = addVictoryGold();

    if (_siege) {
        /* Check occupier of the castle. */
        if (State::castle_occupants[_castleId] != 0x7F) {
            int villain = State::castle_occupants[_castleId];

            /* The villain we beat was our contracted one. */
            if (State::contract == villain) {
                State::gold += kVillainRewards[State::contract];
                _btVictoryVsVillain->setString(fmt::format(kSiegeVictoryMessage, kShortHeroNames[State::hero], bty::numberK(goldTotal), kVillains[villain][0], kVillainRewards[villain]));
                _engine.getGUI().pushDialog(_dlgVictoryVsVillain);
                State::villains_captured[State::contract] = true;
                State::contract = 17;
            }
            else {
                _btVictoryVsVillain->setString(fmt::format(kSiegeVictoryMessageNoContract, kShortHeroNames[State::hero], bty::numberK(goldTotal), kVillains[villain][0]));
                _engine.getGUI().pushDialog(_dlgVictoryVsVillain);
                relocateVillain(villain);
            }
        }
        else {
            /* The castle we sieged was occupied by monsters. */
            _btVictoryVsMobs->setString(fmt::format(kEncounterVictoryMessage, kShortHeroNames[State::hero], bty::numberK(goldTotal)));
            /* Make sure we count it as a siege win though. */
            _dlgVictoryVsMobs.bind(Key::Enter, [this](int) {
                _engine.getGUI().popDialog();
                _engine.winSiegeBattle(_castleId);
            });
            _dlgVictoryVsMobs.bind(Key::Backspace, [this](int) {
                _engine.getGUI().popDialog();
                _engine.winSiegeBattle(_castleId);
            });
            _engine.getGUI().pushDialog(_dlgVictoryVsMobs);
        }
    }
    else {
        _btVictoryVsMobs->setString(fmt::format(kEncounterVictoryMessage, kShortHeroNames[State::hero], bty::numberK(goldTotal)));
        /* Make sure we count it as an encounter win. */
        _dlgVictoryVsMobs.bind(Key::Enter, [this](int) {
            _engine.getGUI().popDialog();
            _engine.winEncounterBattle(_mobId);
        });
        _dlgVictoryVsMobs.bind(Key::Backspace, [this](int) {
            _engine.getGUI().popDialog();
            _engine.winEncounterBattle(_mobId);
        });
        _engine.getGUI().pushDialog(_dlgVictoryVsMobs);
    }

    for (int i = 0; i < 5; i++) {
        State::army[i] = _armies[0][i];
        State::counts[i] = _unitStates[0][i].count;
    }
}

void Battle::battleDefeat()
{
    for (int i = 0; i < 5; i++) {
        (*_extEnemyArmy)[i] = _armies[1][i];
        (*_extEnemyCounts)[i] = _unitStates[1][i].count;
    }
    _engine.loseBattle();
}

bool Battle::boardAnyEnemyAround() const
{
    return boardAnyEnemyAround(_curUnit.x, _curUnit.y);
}

bool Battle::boardAnyEnemyAround(int team, int unit) const
{
    const auto &u = battleGetUnit();
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int x = _unitStates[team][unit].x - 1 + i;
            int y = _unitStates[team][unit].y - 1 + j;
            if (x < 0 || x > 5 || y < 0 || y > 4) {
                continue;
            }
            if (x == _unitStates[team][unit].x && y == _unitStates[team][unit].y) {
                continue;
            }
            auto [unit, enemy] = boardGetUnitAt(x, y);

            if (unit != -1 && u.outOfControl) {
                return true;
            }

            if (enemy) {
                return enemy;
            }
        }
    }
    return false;
}

bool Battle::boardTileBlocked(int x, int y) const
{
    if (x < 0 || x > 5 || y < 0 || y > 4) {
        return true;
    }
    return _terrain[x + y * 6] != 0 || std::get<0>(boardGetUnitAt(x, y)) != -1;
}

bool Battle::boardBlocked() const
{
    return boardBlocked(_curUnit.x, _curUnit.y);
}

bool Battle::boardBlocked(int team, int unit) const
{
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int x = _unitStates[team][unit].x - 1 + i;
            int y = _unitStates[team][unit].y - 1 + j;
            if (x < 0 || x > 5 || y < 0 || y > 4) {
                continue;
            }
            if (x == _unitStates[team][unit].x && y == _unitStates[team][unit].y) {
                continue;
            }
            if (!boardTileBlocked(x, y)) {
                return false;
            }
        }
    }
    return true;
}

void Battle::pauseShow()
{
    static constexpr const char *const kBattleOptions[] = {
        "View your army",
        "View your character",
        "Use magic",
        "Pass",
        "Wait",
        "Game controls",
        "Give up",
    };

    auto dialog = _engine.getGUI().makeDialog(8, 9, 24, 12);
    for (int i = 0; i < sizeof(kBattleOptions) / sizeof(kBattleOptions)[0]; i++) {
        dialog->addOption(3, 2 + i, kBattleOptions[i]);
    }
    dialog->onKey([this, dialog](Key key) {
        switch (key) {
            case Key::Backspace:
                _engine.getGUI().popDialog();
                break;
            case Key::Enter:
                _engine.getGUI().popDialog();
                uiConfirmMenu(dialog->getSelection());
                break;
            default:
                return false;
        }
        return true;
    });
}

void Battle::pauseGiveUp()
{
    static constexpr const char *const kGiveUpPrompt = {
        "   Giving up will\n"
        " forfeit your armies\n"
        "and send you back to\n"
        "      the King.",
    };

    auto dialog = _engine.getGUI().makeDialog(9, 10, 22, 9);
    dialog->addString(1, 1, kGiveUpPrompt);
    dialog->addOption(4, 6, "Continue battle");
    dialog->addOption(4, 7, "Give up");
    dialog->onKey([this, dialog](Key key) {
        switch (key) {
            case Key::Backspace:
                _engine.getGUI().popDialog();
                break;
            case Key::Enter:
                _engine.getGUI().popDialog();
                if (dialog->getSelection() == 1) {
                    battleDefeat();
                }
                break;
            default:
                return false;
        }
        return true;
    });
}

void Battle::pauseUseMagic()
{
    if (_usedSpellThisTurn) {
        uiSetStatus("You may only cast one spell per round!", true);
        return;
    }

    SceneMan::instance().setScene("usemagic");
}

void Battle::uiSetStatus(const std::string &msg, bool waitForEnter)
{
    if (waitForEnter) {
        _engine.getGUI().getHUD().setError(msg);
    }
    else {
        _engine.getGUI().getHUD().setTitle(msg);
    }
}

const Battle::UnitState &Battle::battleGetUnit() const
{
    return _unitStates[_curUnit.x][_curUnit.y];
}

Battle::UnitState &Battle::battleGetUnit()
{
    return _unitStates[_curUnit.x][_curUnit.y];
}

void Battle::afnTryShoot(Action action)
{
    auto [unit, enemy] = boardGetUnitAt(action.to.x, action.to.y);
    if (unit == -1) {
        uiSetStatus("      You must target somebody!");
    }
    else {
        if (enemy) {
            battleDoAction({.id = AidShootAttack, .from = action.from, .to = {!_curUnit.x, unit}});
        }
        else if (unit == _curUnit.y) {
            battleDoAction({AidWait, action.from});
        }
        else {
            uiSetStatus("   You must select an opposing army!");
        }
    }
}

void Battle::uiSetCursorMode(Cursor c)
{
    _cursorMode = c;
    switch (c) {
        case Cursor::Fly:
            _spCursor.setTexture(_texCursorMove);
            _cursorConstrained = false;
            break;
        case Cursor::Move:
            _spCursor.setTexture(_texCursorMove);
            _cursorConstrained = true;
            break;
        case Cursor::Melee:
            _spCursor.setTexture(_texCursorMelee);
            _cursorConstrained = true;
            break;
        case Cursor::Shoot:
            _spCursor.setTexture(_texCursorShoot);
            _cursorConstrained = false;
            break;
        case Cursor::Magic:
            _spCursor.setTexture(_texCursorMagic);
            _cursorConstrained = false;
            break;
        default:
            _spCursor.setTexture(nullptr);
            break;
    }
}

void Battle::aiMakeAction()
{
    auto &unit = battleGetUnit();

    uiUpdateCurrentUnit();

    if (unit.flying) {
        bool moved {false};

        std::vector<int> triedUnits;

        int tries = 0;
        int target = -1;
        while (!moved && tries < 5) {
            tries++;

            int team = 0;
            int target = battleGetRangedUnit(&team);

            /* No ranged units found, find the lowest HP unit. */
            if (target == -1) {
                target = battleGetLowestHpUnit(&team);
                if (team == _curUnit.x && target == _curUnit.y) {
                    target = -1;
                }
            }

            if (target != -1) {
                bool triedUnit {false};
                for (int j = 0; j < triedUnits.size(); j++) {
                    if (triedUnits[j] == target) {
                        triedUnit = true;
                        break;
                    }
                }

                if (!triedUnit) {
                    triedUnits.push_back(target);
                }
                else {
                    continue;
                }
            }
            else {
                continue;
            }

            /* Find an unoccupied adjacent tile. */
            glm::ivec2 tile = boardGetAdjacentTile(target);
            if (tile.x != -1 && tile.y != -1) {
                uiSetStatus(fmt::format("{} fly", battleGetName()));
                battleDoAction({.id = AidTryMove, .from = _curUnit, .to = {tile.x, tile.y}});
                moved = !unit.flying;
                battleDelayThen(nullptr);
            }
        }

        /* Couldn't find a target. */
        if (!moved) {
            spdlog::warn("Couldn't find a fly target!");
        }
    }
    /* Can shoot */
    else if (unit.ammo > 0 && !boardAnyEnemyAround()) {
        int team = 0;
        int target = battleGetRangedUnit(&team);
        if (target == -1) {
            target = battleGetLowestHpUnit(&team);
            if (team == _curUnit.x && target == _curUnit.y) {
                target = -1;
            }
        }
        auto tile = boardGetAdjacentTile(target);
        if (tile.x == -1 && tile.y == -1) {
            spdlog::warn("Couldn't find a shoot target!");
        }
        else {
            battleDoAction({AidShootAttack, _curUnit, {team, target}});
        }
    }
    /* Melee range */
    else {
        /* Attack lowest HP adjacent unit. */
        if (boardAnyEnemyAround()) {
            int team = 0;
            int lowestHpUnit = -1;
            int lowestHp = std::numeric_limits<int>::max();
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    int x = _unitStates[_curUnit.x][_curUnit.y].x - 1 + i;
                    int y = _unitStates[_curUnit.x][_curUnit.y].y - 1 + j;
                    if (x < 0 || x > 5 || y < 0 || y > 4) {
                        continue;
                    }
                    if (_terrain[x + y * 6] != 0) {
                        continue;
                    }
                    auto [index, enemy] = boardGetUnitAt(x, y);
                    if (index == -1 || (!enemy && !unit.outOfControl)) {
                        continue;
                    }
                    team = enemy ? _curUnit.x == 0 ? 1 : 0 : _curUnit.x;
                    if (team == _curUnit.x && index == _curUnit.y) {
                        continue;
                    }
                    if (_unitStates[team][index].hp <= lowestHp) {
                        lowestHp = _unitStates[team][index].hp;
                        lowestHpUnit = index;
                    }
                }
            }
            assert(lowestHpUnit != -1);
            battleDoAction({AidMeleeAttack, _curUnit, {team, lowestHpUnit}});
        }
        /* Move towards best target. */
        else {
            if (boardBlocked()) {
                battleDoAction({AidWait, _curUnit});
                return;
            }

            int team = 0;
            int target = battleGetRangedUnit(&team);

            /* No ranged units found, find the lowest HP unit. */
            if (target == -1) {
                target = battleGetLowestHpUnit(&team);
            }

            const auto &targetUnit = _unitStates[team][target];
            const auto &curUnit = _unitStates[_curUnit.x][_curUnit.y];

            int x = curUnit.x;
            int y = curUnit.y;

            std::array<glm::ivec2, 8> tiles = {{
                {x - 1, y - 1},
                {x, y - 1},
                {x + 1, y - 1},
                {x - 1, y},
                {x + 1, y},
                {x - 1, y + 1},
                {x, y + 1},
                {x + 1, y + 1},
            }};

            std::array<std::pair<int, int>, 8> distances;

            for (int i = 0; i < 8; i++) {
                distances[i].first = i;
                if (boardTileBlocked(tiles[i].x, tiles[i].y)) {
                    distances[i].second = 0xFF;
                }
                else {
                    distances[i].second = std::abs(tiles[i].x - targetUnit.y) + std::abs(tiles[i].y - targetUnit.y);
                }

                if (_engine.getGameOptions().debug) {
                    if (tiles[i].x == -1 || tiles[i].y == -1) {
                        _costSquares[i].setPosition({-100, -100});
                    }
                    else {
                        _costSquares[i].setPosition({16.0f + tiles[i].x * 48.0f, 24.0f + tiles[i].y * 40.0f});
                    }
                    if (distances[i].second <= 1) {
                        _costSquares[i].setColor({0.203, 0.835, 0.247, 0.8});
                    }
                    else if (distances[i].second == 2) {
                        _costSquares[i].setColor({0.203, 0.631, 0.835, 0.8});
                    }
                    else if (distances[i].second == 3) {
                        _costSquares[i].setColor({0.980, 0.447, 0, 0.8});
                    }
                    else {
                        _costSquares[i].setColor({0.5f + distances[i].second * 0.08f, 0.8f - distances[i].second * 0.1f, 0.023, 0.8});
                    }
                }
            }

            /* We want to head towards the tile with the lowest distance to target, i.e. closest to target. */
            std::sort(distances.begin(), distances.end(), [](const std::pair<int, int> &a, const std::pair<int, int> &b) {
                return a.second < b.second;
            });

            int nextX = -1;
            int nextY = -1;

            for (int i = 0; i < 8; i++) {
                /* When we reach 0xFF's, the rest of the array is only these so we can break. */
                if (distances[i].second == 0xFF) {
                    break;
                }
                nextX = tiles[distances[i].first].x;
                nextY = tiles[distances[i].first].y;
                break;
            }

            if (nextX != -1 && nextY != -1) {
                uiSetStatus(fmt::format("{} Move", battleGetName()));
                battleDoAction({.id = AidTryMove, .from = _curUnit, .to = {nextX, nextY}, .nextUnit = false});
                battleDelayThen([this, unit]() {
                    if (unit.moves == 0) {
                        battleOnMove();
                    }
                });
            }
        }
    }
}

int Battle::battleGetRangedUnit(int *team_) const
{
    int teamA = -1;

    if (battleGetUnit().outOfControl) {
        teamA = 0;
    }
    else {
        teamA = !_curUnit.x;
    }

    int target = -1;
    int most_ranged_damage = 0;

    for (int team = teamA; team < teamA + 1; team++) {
        for (int i = 0; i < 5; i++) {
            if (_armies[team][i] == -1 || (team == _curUnit.x && i == _curUnit.y)) {
                continue;
            }
            if (_unitStates[team][i].ammo) {
                if (kUnits[_armies[team][i]].shootMaxDmg * _unitStates[team][i].count >= most_ranged_damage) {
                    target = i;
                    if (team_) {
                        *team_ = team;
                    }
                }
            }
        }
    }

    return target;
}

glm::ivec2 Battle::boardGetAdjacentTile(int player_unit) const
{
    int teamA = -1;

    if (battleGetUnit().outOfControl) {
        teamA = 0;
    }
    else {
        teamA = !_curUnit.x;
    }

    glm::ivec2 tile {-1, -1};

    for (int team = teamA; team < teamA + 1; team++) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                int x = _unitStates[team][player_unit].x - 1 + i;
                int y = _unitStates[team][player_unit].y - 1 + j;
                if (x < 0 || x > 5 || y < 0 || y > 4) {
                    continue;
                }
                if (_terrain[x + y * 6] != 0) {
                    continue;
                }
                auto [index, enemy] = boardGetUnitAt(x, y);
                if (index != -1) {
                    continue;
                }
                tile.x = x;
                tile.y = y;
                break;
            }
        }
    }

    return tile;
}

int Battle::battleGetLowestHpUnit(int *team_) const
{
    int teamA = -1;

    if (battleGetUnit().outOfControl) {
        teamA = 0;
    }
    else {
        teamA = !_curUnit.x;
    }

    int target = -1;
    int minHp = std::numeric_limits<int>::max();

    for (int team = teamA; team < teamA + 1; team++) {
        for (int i = 0; i < 5; i++) {
            if (_armies[team][i] == -1) {
                continue;
            }

            if (_unitStates[team][i].hp <= minHp) {
                minHp = _unitStates[team][i].hp;
                target = i;
                if (team_) {
                    *team_ = team;
                }
            }
        }
    }

    return target;
}

bool Battle::battleNotUser() const
{
    return _curUnit.x == 1 || (_curUnit.x == 0 && _unitStates[0][_curUnit.y].outOfControl);
}

bool Battle::battleCanShoot() const
{
    return battleGetUnit().ammo > 0 && !boardAnyEnemyAround();
}

void Battle::uiUpdateCount(int team, int unit)
{
    auto count = std::to_string(_unitStates[team][unit].count);
    /* X 16 = 8 for border, 8 for board offset */
    float unitX = 16.0f + _unitStates[team][unit].x * 48.0f - 8;
    /* Y 16 = 16 for border */
    float unitY = 16.0f + _unitStates[team][unit].y * 40.0f;
    float countX = unitX + 48.0f - count.size() * 8.0f;
    float countY = unitY + 40.0f - 8.0f;
    _btCounts[team][unit].setString(count);
    _btCounts[team][unit].setPosition(countX, countY);
}

void Battle::startSiegeBattle(int castleId)
{
    _siege = true;
    _castleId = castleId;
    _extEnemyArmy = &State::castle_armies[castleId];
    _extEnemyCounts = &State::castle_counts[castleId];

    _spBg.setTexture(_texSiegeBg);

    /* clang-format off */
	_terrain = {{
		1, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 1,
		1, 1, 0, 0, 1, 1,
	}};
    /* clang-format on */

    initArmies();
    initBattle();
}

void Battle::startEncounterBattle(int mobId)
{
    _siege = false;
    _mobId = mobId;
    _extEnemyArmy = &State::mobs[State::continent][mobId % 40].army;
    _extEnemyCounts = &State::mobs[State::continent][mobId % 40].counts;

    _spBg.setTexture(_texEncounterBg);

    /* clang-format off */
	_terrain = {{
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
	}};
    /* clang-format on */

    generateTerrain();

    initArmies();
    initBattle();
}

void Battle::generateTerrain()
{
    std::vector<glm::ivec2> obstacles;
    int num_obstacles = bty::random(0, 6);
    std::unordered_map<int, int> tilesOnX;
    for (int i = 0; i < num_obstacles; i++) {
    regen:
        glm::ivec2 tile {bty::random(1, 4), bty::random(0, 4)};
        if (tilesOnX[tile.x] == 4) {
            goto regen;
        }
        tilesOnX[tile.x]++;
        for (int j = 0; j < obstacles.size(); j++) {
            if (tile == obstacles[j]) {
                goto regen;
            }
        }
        obstacles.push_back(tile);
    }

    for (const auto &tile : obstacles) {
        _terrain[tile.x + tile.y * 6] = bty::random(1, 3);
    }

    int obstacleIdx = 0;
    for (int x = 0; x < 6; x++) {
        for (int y = 0; y < 5; y++) {
            int id = _terrain[x + y * 6];
            if (id == 0) {
                continue;
            }
            _spTerrain[obstacleIdx].setTexture(_texObstacles[id - 1]);
            if (id == 3) {
                _spTerrain[obstacleIdx].setPosition({16.0f + 48.0f * x, 24.0f + 40.0f * y});
            }
            else {
                _spTerrain[obstacleIdx].setPosition({16.0f + 48.0f * x, 24.0f + 40.0f * y});
            }
            obstacleIdx++;
        }
    }
}

void Battle::initArmies()
{
    int *armies[] = {State::army.data(), _extEnemyArmy->data()};
    int *counts[] = {State::counts.data(), _extEnemyCounts->data()};

    /* Set armies from shared State:: */
    /* Initialise states */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            _armies[i][j] = armies[i][j];
            const auto &unit = kUnits[armies[i][j]];
            auto &us = _unitStates[i][j];
            us.id = armies[i][j];
            us.startCount = counts[i][j];
            us.turnCount = counts[i][j];
            us.count = counts[i][j];
            us.hp = unit.hp;
            us.injury = 0;
            us.ammo = unit.initialAmmo;
            us.outOfControl = i == 0 && (us.hp * us.count) > State::leadership;
            us.flying = !!(unit.abilities & AbilityFly);
            us.moves = unit.initialMoves;
            us.waits = 0;
            us.frozen = false;
            us.retaliated = false;
        }
    }

    static const glm::ivec2 kStartingPositions[2][2][5] = {
        {
            // Encounter
            {
                // Team 0
                {0, 0},
                {0, 1},
                {0, 2},
                {0, 3},
                {0, 4},
            },
            {
                // Team 1
                {5, 0},
                {5, 1},
                {5, 2},
                {5, 3},
                {5, 4},
            },
        },
        {
            // Siege
            {
                // Team 0
                {2, 4},
                {3, 4},
                {1, 3},
                {2, 3},
                {3, 3},
            },
            {
                // Team 1
                {1, 0},
                {2, 0},
                {3, 0},
                {4, 0},
                {2, 1},
            },
        },
    };

    int type = static_cast<int>(_siege);    // encounter

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (_armies[i][j] == -1) {
                continue;
            }

            /* Set sprites */
            boardMoveUnitTo(i, j, kStartingPositions[type][i][j].x, kStartingPositions[type][i][j].y);
            _spUnits[i][j].setTexture(_texUnits[_armies[i][j]]);
            if (i == 1) {
                _spUnits[i][j].setFlip(true);
            }

            /* Set counts */
            uiUpdateCount(i, j);
        }
    }
}

void Battle::initBattle()
{
    /* Can happen when the previous battle ended in a draw. */
    /* The battle immediately ends and the player wins. */
    if (battleEnd()) {
        return;
    }

    _curUnit = {0, 0};
    uiUpdateState();
}

int Battle::addVictoryGold()
{
    int goldTotal = 0;
    for (int i = 0; i < 5; i++) {
        auto &us = _unitStates[1][i];
        goldTotal += us.startCount * kUnits[us.id].weeklyCost * 5;
    }

    int a = goldTotal;
    goldTotal += bty::random(10);
    goldTotal += bty::random(a / 8);
    State::gold += goldTotal;

    return goldTotal;
}

void Battle::relocateVillain(int villainId)
{
    std::vector<int> potentialCastles;

    /* Relocate the villain. */
    for (int i = 0; i < 26; i++) {
        if (kCastleInfo[i].continent != State::continent) {
            continue;
        }
        if (State::castle_occupants[i] == 0x7F) {
            potentialCastles.push_back(i);
        }
    }

    /* No available castles, steal one from the player. Sorry! */
    if (potentialCastles.size() == 0) {
        for (int i = 0; i < 26; i++) {
            if (kCastleInfo[i].continent != State::continent) {
                continue;
            }
            /* At least prioritise empty ones, ideally not *this* one. */
            if (State::castle_occupants[i] == -1 && i != _castleId) {
                potentialCastles.push_back(i);
            }
        }
    }

    int new_castle = -1;

    /* Use the current castle. */
    if (potentialCastles.size() == 0) {
        new_castle = _castleId;
    }
    else {
        new_castle = potentialCastles[rand() % potentialCastles.size()];
    }

    State::castle_occupants[new_castle] = villainId;

    /* And their army. */
    genVillainArmy(villainId, State::castle_armies[new_castle], State::castle_counts[new_castle]);
}

bool Battle::battleEnd()
{
    int winner = checkWinner();
    if (winner == -1) {
        return false;
    }
    else if (winner == 0) {
        battleVictory();
    }
    else {
        battleDefeat();
    }
    return true;
}
