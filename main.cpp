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

    SDL_Joystick * joy = nullptr;
    SDL_GameController * ctrl = nullptr;

    bool running = true;
    while(running)
    {
        SDL_Event ev;
        while(SDL_PollEvent(&ev)) // TODO: change to SDL_WaitEvent
        {
            switch(ev.type)
            {
                case SDL_QUIT:
                case SDL_KEYDOWN:
                    running = false;
                    break;
                case SDL_JOYDEVICEADDED:
                // case SDL_CONTROLLERDEVICEADDED:
                    std::cout<<"Joydevice added "<<ev.jdevice.which<<"\n";
                    if(SDL_IsGameController(ev.jdevice.which))
                    {
                        std::cout<<"Created as gamecontroller\n";
                        ctrl = SDL_GameControllerOpen(ev.jdevice.which);
                        if(!ctrl)
                            std::cerr<<"bad ctrl: "<<SDL_GetError()<<'\n';
                    }
                    else
                    {
                        std::cout<<"Created as joystick\n";
                        joy = SDL_JoystickOpen(ev.jdevice.which);
                        if(!joy)
                            std::cerr<<"bad joy: "<<SDL_GetError()<<'\n';
                    }
                    break;
                case SDL_JOYDEVICEREMOVED:
                    std::cout<<"Joydevice removed "<<ev.jdevice.which<<"\n";
                    if(joy)
                    {
                        SDL_JoystickClose(joy);
                        joy = nullptr;
                    }
                    break;
                case SDL_CONTROLLERDEVICEREMOVED:
                    std::cout<<"Ctrldevice removed "<<ev.cdevice.which<<"\n";
                    if(ctrl)
                    {
                        SDL_GameControllerClose(ctrl);
                        ctrl = nullptr;
                    }
                    break;
                case SDL_JOYAXISMOTION:
                    std::cout<<"Joyaxismotion: "<<ev.jaxis.which<<' '<<(int)ev.jaxis.axis<<' '<<ev.jaxis.value<<'\n';
                    break;
                case SDL_JOYBUTTONDOWN:
                    std::cout<<"Joybutton: "<<ev.jbutton.which<<' '<<(int)ev.jbutton.button<<' '<<(int)ev.jbutton.state<<'\n';
                    break;
                case SDL_JOYHATMOTION:
                    std::cout<<"Joyhat: "<<ev.jhat.which<<' '<<(int)ev.jhat.hat<<' '<<(int)ev.jhat.value<<'\n';
                    break;
                case SDL_CONTROLLERAXISMOTION:
                    std::cout<<"caxismotion: "<<ev.caxis.which<<' '<<(int)ev.caxis.axis<<' '<<(int)ev.caxis.value<<'\n';
                    break;
                case SDL_CONTROLLERBUTTONDOWN:
                    std::cout<<"Cbutton: "<<(int)ev.cbutton.which<<' '<<(int)ev.cbutton.button<<' '<<(int)ev.cbutton.state<<'\n';
                    break;
                case SDL_CONTROLLERDEVICEREMAPPED:
                    std::cout<<"Cdevice remapped "<<ev.cdevice.which<<"\n";
                    break;

                default:
                    break;
            }
        }

        std::cout.flush();
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, tex, nullptr, nullptr);
        text.render(renderer, 0, 0);
        SDL_RenderPresent(renderer);
    }

    if(joy)
        SDL_JoystickClose(joy);

    if(ctrl)
        SDL_GameControllerClose(ctrl);

    return 0;
}
