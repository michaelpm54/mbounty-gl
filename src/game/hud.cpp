#include "game/hud.hpp"

#include <spdlog/spdlog.h>

#include "assets.hpp"
#include "bounty.hpp"
#include "gfx/font.hpp"
#include "gfx/gfx.hpp"
#include "shared-state.hpp"

void Hud::load(bty::Assets &assets, SharedState &state)
{
    state_ = &state;

    blank_frame_ = assets.get_texture("frame/game-empty.png");
    hud_frame_ = assets.get_texture("frame/game-hud.png");

    frame_.set_texture(hud_frame_);

    top_bar_.set_size(304, 9);
    top_bar_.set_position({8, 7});

    top_bar_.set_color(bty::get_box_color(state.difficulty_level));

    font_ = &assets.get_font();
    name_.create(1, 1, kHeroNames[state.hero_id][state.hero_rank], *font_);
    days_.create(26, 1, "", *font_);

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

    update_state();
}

void Hud::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    gfx.draw_sprite(frame_, camera);
    gfx.draw_rect(top_bar_, camera);
    gfx.draw_text(name_, camera);
    gfx.draw_text(days_, camera);
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

void Hud::update_state()
{
    contract_.set_texture(contract_textures_[state_->contract]);
    name_.set_string(kHeroNames[state_->hero_id][state_->hero_rank]);
    days_.set_string(fmt::format("Days Left:{}", state_->days));
    siege_.set_texture(state_->siege ? siege_yes : siege_no);
    magic_.set_texture(state_->magic ? magic_yes : magic_no);
    for (int i = 0; i < 17; i++) {
        hide_piece_[kPuzzleVillainPositions[i]] = state_->villains_caught[i];
    }
    for (int i = 0; i < 8; i++) {
        hide_piece_[kPuzzleArtifactPositions[i]] = state_->artifacts_found[i];
    }
    int gold = state_->gold;
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
    contract_.animate(dt);
    siege_.animate(dt);
    magic_.animate(dt);
}

void Hud::set_title(const std::string &msg)
{
    name_.set_string(msg);
    days_.set_string("");
}

bty::Sprite *Hud::get_contract()
{
    return &contract_;
}

void Hud::set_blank_frame()
{
    frame_.set_texture(blank_frame_);
}

void Hud::set_hud_frame()
{
    frame_.set_texture(hud_frame_);
}

void Hud::set_color(bty::BoxColor color)
{
    top_bar_.set_color(color);
}
