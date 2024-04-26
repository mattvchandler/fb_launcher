#include <iostream>
#include <list>
#include <stdexcept>

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

    auto sdl_lib = SDL::SDL{SDL_INIT_VIDEO | SDL_INIT_JOYSTICK};
    auto ttf_lib = SDL::TTF{};

    auto window = SDL::Window{"fb_launcher"};
    auto renderer = SDL::Renderer(window);

    auto tex = SDL::Texture(renderer, argv[1]);
    auto font = SDL::Font("sans-serif", 20);

    auto text = font.render_text(renderer, "Testing this\nText", SDL_Color{0xFF, 0xFF, 0xFF, 0xFF});

    auto joysticks = std::list<SDL::Joystick>{};

    bool running = true;
    while(running)
    {
        SDL_Event ev;
        if(SDL_WaitEvent(&ev) < 0)
            SDL::sdl_error("Error getting SDL event");

        switch(ev.type)
        {
            case SDL_QUIT:
            case SDL_KEYDOWN:
                running = false;
                break;

            case SDL_JOYDEVICEADDED:
                joysticks.emplace_back(ev.jdevice.which);
                std::cout<<"Joydevice created "<<ev.jdevice.which<<" "<<SDL_JoystickName(joysticks.back())<<"\n";
                break;
            case SDL_JOYDEVICEREMOVED:
                std::cout<<"Joydevice removed "<<ev.jdevice.which<<"\n";

                for(auto i = std::begin(joysticks); i != std::end(joysticks);)
                {
                    if(SDL_JoystickInstanceID(*i) == ev.jdevice.which)
                    {
                        i = joysticks.erase(i);
                        break;
                    }
                    else
                        ++i;
                }
                break;

            // TODO: also enter
            case SDL_JOYBUTTONDOWN: // TODO: all buttons launch the selected app
                std::cout<<"Joybutton: "<<ev.jbutton.which<<' '<<(int)ev.jbutton.button<<' '<<(int)ev.jbutton.state<<'\n';
                break;

            // TODO: next / prev actions
            // TODO: keyboard arrows
            // TODO: axis deadzone
            case SDL_JOYAXISMOTION:
                std::cout<<"Joyaxismotion: "<<ev.jaxis.which<<' '<<(int)ev.jaxis.axis<<' '<<ev.jaxis.value<<'\n';
                break;
            case SDL_JOYHATMOTION:
                std::cout<<"Joyhat: "<<ev.jhat.which<<' '<<(int)ev.jhat.hat<<' '<<(int)ev.jhat.value<<'\n';
                break;

            default:
                break;
        }

        std::cout.flush();
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, tex, nullptr, nullptr);
        text.render(renderer, 0, 0);
        SDL_RenderPresent(renderer);
    }

    return 0;
}
