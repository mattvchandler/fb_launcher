#include <iostream>


#include "font.hpp"
#include "sdl.hpp"

// TODO: keyboard / joystick input
// TODO: CEC input?
// TODO: load / parse config file
// TODO: unload everything and launch selected program
// TODO: power-off, reboot options
// TODO: autolaunch 1st app (Kodi)


int main(int argc, char * argv[])
{
    if(argc < 2)
    {
        std::cerr<<"Must specify PNG image to display\n";
        return 1;
    }

    auto sdl_lib = SDL::SDL{SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER};
    auto ttf_lib = SDL::TTF{};

    auto window = SDL::Window{"fb_launcher"};
    auto renderer = SDL::Renderer(window);

    auto tex = SDL::Texture(renderer, argv[1]);
    auto font = SDL::Font("sans-serif", 20);

    auto text = font.render_text(renderer, "Testing this\nText", SDL_Color{0xFF, 0xFF, 0xFF, 0xFF});
    auto text_dest = SDL_Rect{0, 0, text.get_width(), text.get_height()};

    bool running = true;
    while(running)
    {
        SDL_Event ev;
        while(SDL_PollEvent(&ev))
        {
            switch(ev.type)
            {
                case SDL_QUIT:
                case SDL_KEYDOWN:
                    running = false;
                    break;
                default:
                    break;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, tex, nullptr, nullptr);
        SDL_RenderCopy(renderer, text, nullptr, &text_dest);
        SDL_RenderPresent(renderer);
    }

    return 0;
}
