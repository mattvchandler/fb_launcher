#ifndef MENU_HPP
#define MENU_HPP

#include <chrono>
#include <map>

#include "app.hpp"
#include "cec.hpp"
#include "font.hpp"
#include "joystick.hpp"
#include "sdl.hpp"
#include "texture.hpp"

class Menu
{
public:
    Menu(const std::vector<App> & apps);
    int run();
    int get_exited() const { return exited_; }

private:
    const std::vector<App> & apps_;

    bool running_ {false};
    bool exited_ {false};
    int index_ {0u};

    std::chrono::system_clock::time_point animation_start_ {};
    int animation_direction_ {0};

    int w_{0}, h_{0};

    SDL::SDL sdl_lib_{SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER};
    SDL::TTF ttf_lib_;
    SDL::Window window_{"fb_launcher"};
    SDL::Renderer renderer_{window_};
    std::map<int, SDL::Joystick> joysticks;
    Uint32 animation_event_;

    SDL::Texture mouse_icon_ {renderer_, "input_icons/computer-mouse.svg", 32, 32, 0xFF, 0xFF, 0xFF};
    SDL::Texture keyboard_icon_ {renderer_, "input_icons/keyboard.svg", 32, 32, 0xFF, 0xFF, 0xFF};
    SDL::Texture gamepad_icon_ {renderer_, "input_icons/gamepad.svg", 32, 32, 0xFF, 0xFF, 0xFF};
    SDL::Texture cec_icon_ {renderer_, "input_icons/mobile-retro.svg", 32, 32, 0xFF, 0xFF, 0xFF};

    CEC_Input cec_;

    struct Menu_textures
    {
        SDL::Texture title;
        SDL::Texture desc;
        SDL::Texture thumbnail;
    };
    std::vector<Menu_textures> app_textures_;

    void prev();
    void next();
    void select();

    void resize(int w, int h);

    void draw();
    void draw_row(int pos);
};

#endif // MENU_HPP
