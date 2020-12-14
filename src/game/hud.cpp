#include "game/hud.hpp"

#include <spdlog/spdlog.h>

#include "assets.hpp"
#include "bounty.hpp"
#include "gfx/font.hpp"
#include "gfx/gfx.hpp"

Hud::Hud(bty::Assets &assets)
{
    blank_frame_ = assets.get_texture("frame/game-empty.png");
    hud_frame_ = assets.get_texture("frame/game-hud.png");

    frame_.set_texture(hud_frame_);

    top_bar_.set_size(304, 9);
    top_bar_.set_position({8, 7});

    const auto &font = assets.get_font();
    name_.create(1, 1, "", font);
    days_.create(26, 1, "", font);
    error_text.create(1, 1, "", font);
    timestop_string_.create(26, 1, "", font);

    contract_textures_.resize(18);
    for (int i = 0, max = static_cast<int>(contract_textures_.size() - 1); i < max; i++) {
        contract_textures_[i] = assets.get_texture(fmt::format("villains/{}.png", i), {4, 1});
    }
    contract_textures_.back() = assets.get_texture("villains/empty.png");
    contract_.set_texture(contract_textures_[0]);
    contract_.set_position({262, 24});

    siege_no = assets.get_texture("hud/siege-no.png");
    siege_yes = assets.get_texture("hud/siege-yes.png", {4, 1});
    siege_.set_position({262, 64});

    magic_no = assets.get_texture("hud/magic-no.png");
    magic_yes = assets.get_texture("hud/magic-yes.png", {4, 1});
    magic_.set_position({262, 104});

    puzzle_.set_texture(assets.get_texture("hud/puzzle-bg.png"));
    puzzle_.set_position({262, 144});

    money_.set_texture(assets.get_texture("hud/gold-bg.png"));
    money_.set_position({262, 184});

    const auto *piece_texture = assets.get_texture("hud/puzzle-piece.png");

    int p = 0;

    float x = 264;
    float y = 145;
    for (int j = 0; j < 5; j++) {
        for (int i = 0; i < 5; i++) {
            auto &piece = pieces_[p++];
            piece.set_position(x + i * 8, y + j * 6);
            piece.set_texture(piece_texture);
        }
    }

    const auto *gold_tex = assets.get_texture("hud/gold-2-gold.png");
    const auto *silver_tex = assets.get_texture("hud/gold-1-silver.png");
    const auto *copper_tex = assets.get_texture("hud/gold-0-copper.png");

    for (int i = 0; i < 10; i++) {
        gold_[i].set_texture(gold_tex);
        gold_[i].set_position(264, 208 - i * 2);
    }
    for (int i = 0; i < 10; i++) {
        gold_[10 + i].set_texture(silver_tex);
        gold_[10 + i].set_position(280, 208 - i * 2);
    }
    for (int i = 0; i < 10; i++) {
        gold_[20 + i].set_texture(copper_tex);
        gold_[20 + i].set_position(296, 208 - i * 2);
    }
}

void Hud::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    gfx.draw_sprite(frame_, camera);
    gfx.draw_rect(top_bar_, camera);
    if (error) {
        gfx.draw_text(error_text, camera);
    }
    else {
        gfx.draw_text(name_, camera);
        if (timestop_) {
            gfx.draw_text(timestop_string_, camera);
        }
        else {
            gfx.draw_text(days_, camera);
        }
    }
    if (no_sprites) {
        return;
    }
    gfx.draw_sprite(contract_, camera);
    gfx.draw_sprite(siege_, camera);
    gfx.draw_sprite(magic_, camera);
    gfx.draw_sprite(puzzle_, camera);
    gfx.draw_sprite(money_, camera);
    for (int i = 0; i < 25; i++) {
        if (!hide_piece_[i]) {
            gfx.draw_sprite(pieces_[i], camera);
        }
    }
    for (int i = 0; i < num_gold_coins_; i++) {
        gfx.draw_sprite(gold_[i], camera);
    }
    for (int i = 0; i < num_silver_coins_; i++) {
        gfx.draw_sprite(gold_[10 + i], camera);
    }
    for (int i = 0; i < num_copper_coins_; i++) {
        gfx.draw_sprite(gold_[20 + i], camera);
    }
}

void Hud::set_contract(int contract)
{
    contract_.set_texture(contract_textures_[contract]);
}

void Hud::set_days(int days)
{
    days_.set_string(fmt::format("Days Left:{}", days));
}

void Hud::set_magic(bool val)
{
    magic_.set_texture(val ? magic_yes : magic_no);
}

void Hud::set_siege(bool val)
{
    siege_.set_texture(val ? siege_yes : siege_no);
}

void Hud::set_puzzle(bool *villains, bool *artifacts)
{
    for (int i = 0; i < 17; i++) {
        hide_piece_[kPuzzleVillainPositions[i]] = villains[i];
    }
    for (int i = 0; i < 8; i++) {
        hide_piece_[kPuzzleArtifactPositions[i]] = artifacts[i];
    }
}

void Hud::set_title(const std::string &str)
{
    name_.set_string(str);
    days_.set_string("");
}

void Hud::set_gold(int gold)
{
    int num_gold = gold / 10000;
    gold -= (num_gold * 10000);
    int num_silver = gold / 1000;
    gold -= (num_silver * 1000);
    int num_copper = gold / 100;
    num_gold_coins_ = num_gold > 10 ? 10 : num_gold;
    num_silver_coins_ = num_silver > 10 ? 10 : num_silver;
    num_copper_coins_ = num_copper > 10 ? 10 : num_copper;
}

void Hud::update(float dt)
{
    contract_.update(dt);
    siege_.update(dt);
    magic_.update(dt);
}

void Hud::set_error(const std::string &msg, std::function<void()> then)
{
    error_then_ = then;
    error_text.set_string(msg);
    error = true;
}

void Hud::clear_error()
{
    error = false;
    if (error_then_) {
        error_then_();
    }
}

bty::Sprite *Hud::get_contract()
{
    return &contract_;
}

void Hud::set_blank_frame()
{
    frame_.set_texture(blank_frame_);
    no_sprites = true;
}

void Hud::set_hud_frame()
{
    frame_.set_texture(hud_frame_);
    no_sprites = false;
}

void Hud::set_color(bty::BoxColor color)
{
    top_bar_.set_color(color);
}

void Hud::set_timestop(int amount)
{
    timestop_string_.set_string(fmt::format("Timestop:{:>4}", amount));
    timestop_ = true;
}

void Hud::clear_timestop()
{
    timestop_ = false;
}

void Hud::set_hero(int hero, int rank)
{
    name_.set_string(kHeroNames[hero][rank]);
}

bool Hud::get_error() const
{
    return error;
}
