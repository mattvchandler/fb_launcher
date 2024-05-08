#ifndef MENU_HPP
#define MENU_HPP

#include <functional>
#include <map>

#include "cec.hpp"
#include "font.hpp"
#include "joystick.hpp"
#include "sdl.hpp"
#include "texture.hpp"

class Menu
{
public:
    Menu(const std::string & test_image);
    int run();

private:
    SDL::SDL sdl_lib_{SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER};
    SDL::TTF ttf_lib_;
    SDL::Window window_{"fb_launcher"};
    SDL::Renderer renderer_{window_};
    std::map<int, SDL::Joystick> joysticks;

    CEC_Input cec_;

    SDL::Texture test_image_;
    SDL::Font title_font_;
    SDL::Font desc_font_;

    int w_{0}, h_{0};

    void prev();
    void next();
    void select();

    void resize(int w, int h);

    void draw();
    void draw_row(int pos, SDL::Renderer & renderer, SDL::Texture & tex);

};

#endif // MENU_HPP
