#ifndef MENU_HPP
#define MENU_HPP

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

private:

    const std::vector<App> & apps_;

    std::size_t index_ = 0u;

    SDL::SDL sdl_lib_{SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER};
    SDL::TTF ttf_lib_;
    SDL::Window window_{"fb_launcher"};
    SDL::Renderer renderer_{window_};
    std::map<int, SDL::Joystick> joysticks;

    CEC_Input cec_;

    struct Menu_textures
    {
        SDL::Texture title;
        SDL::Texture desc;
        SDL::Texture thumbnail;
        SDL::Texture bg;
    };
    std::vector<Menu_textures> app_textures_;

    // TODO: icons for inputs

    int w_{0}, h_{0};

    void prev();
    void next();
    void select();

    void resize(int w, int h);

    void draw();
    void draw_row(int index);
};

#endif // MENU_HPP
