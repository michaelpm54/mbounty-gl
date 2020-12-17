#include "game/game-controls.hpp"

#include "engine/scene-stack.hpp"
#include "game/game-options.hpp"
#include "window/glfw.hpp"

GameControls::GameControls(bty::SceneStack &ss, bty::Assets &assets, GameOptions &game_options)
    : ss(ss)
    , game_options(game_options)
{
    dialog.create(6, 10, 20, 9, bty::BoxColor::Intro, assets);
    dialog.add_line(4, 1, "Game Control");
    dialog.add_line(4, 2, "____________");
    t_music = dialog.add_option(4, 4, "");
    t_sound = dialog.add_option(4, 5, "");
    t_delay = dialog.add_option(4, 6, "");
    update_options();
}

void GameControls::key(int key, int action)
{
    if (action != GLFW_PRESS) {
        return;
    }

    switch (key) {
        case GLFW_KEY_UP:
            dialog.prev();
            break;
        case GLFW_KEY_DOWN:
            dialog.next();
            break;
        case GLFW_KEY_LEFT:
            if (dialog.get_selection() == 2) {
                game_options.combat_delay = (game_options.combat_delay - 1 + 10) % 10;
            }
            break;
        case GLFW_KEY_RIGHT:
            if (dialog.get_selection() == 2) {
                game_options.combat_delay = (game_options.combat_delay + 1) % 10;
            }
            break;
        case GLFW_KEY_ENTER:
            switch (dialog.get_selection()) {
                case 0:
                    game_options.music = !game_options.music;
                    break;
                case 1:
                    game_options.sound = !game_options.sound;
                    break;
                case 2:
                    game_options.combat_delay = (game_options.combat_delay + 1) % 10;
                    break;
                default:
                    break;
            }
            break;
        case GLFW_KEY_BACKSPACE:
            ss.pop(0);
            break;
        default:
            break;
    }

    update_options();
}

void GameControls::update_options()
{
    t_music->set_string(fmt::format("Music {}", game_options.music ? "on" : "off"));
    t_sound->set_string(fmt::format("Sound {}", game_options.sound ? "on" : "off"));
    t_delay->set_string(fmt::format("Delay {}", game_options.combat_delay));
}

void GameControls::update(float dt)
{
    dialog.update(dt);
}

void GameControls::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    dialog.draw(gfx, camera);
}

void GameControls::set_color(bty::BoxColor color)
{
    dialog.set_color(color);
}

void GameControls::set_battle(bool battle)
{
    dialog.set_position(battle ? 10 : 6, 10);
}
