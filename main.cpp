#include <functional>
#include <iostream>
#include <map>

#include "cec.hpp"
#include "font.hpp"
#include "joystick.hpp"
#include "menu.hpp"
#include "sdl.hpp"
#include "texture.hpp"

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
    SDL_ShowCursor(SDL_DISABLE);

    auto renderer = SDL::Renderer(window);
    auto joysticks = std::map<int, SDL::Joystick>{};

    auto menu = Menu{};

    auto tex = SDL::Texture(renderer, argv[1]);

    // TODO: scale to window size - will need to be reloaded if window changes size
    CEC_Input cec;
    cec.register_up(std::bind(&Menu::prev, &menu));
    cec.register_left(std::bind(&Menu::prev, &menu));
    cec.register_down(std::bind(&Menu::next, &menu));
    cec.register_right(std::bind(&Menu::next, &menu));
    cec.register_select(std::bind(&Menu::select, &menu));

    bool running = true;
    while(running)
    {
        SDL_Event ev;
        if(SDL_WaitEvent(&ev) < 0) // might need to change this to SDL_WaitEventTimeout so CEC inputs can come through - experiment to see if that's the case
            SDL::sdl_error("Error getting SDL event");

        switch(ev.type)
        {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_WINDOWEVENT:
                if(ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    int w, h;
                    SDL_GetRendererOutputSize(renderer, &w, &h);

                    std::cout<<"Window resize event: "<<w<<' '<<h<<'\n';

                    menu.resize(w, h);
                }
                break;

            case SDL_JOYDEVICEADDED:
            {
                auto joy = SDL::Joystick{ev.jdevice.which};
                std::cout<<"Joydevice created "<<ev.jdevice.which<<" "<<joy.name()<<"\n";
                joysticks.emplace(SDL_JoystickGetDeviceInstanceID(ev.jdevice.which), std::move(joy));
                break;
            }

            case SDL_JOYDEVICEREMOVED:
                std::cout<<"Joydevice removed "<<ev.jdevice.which<<"\n";
                joysticks.erase(ev.jdevice.which);
                break;

            case SDL_KEYDOWN:
                switch(ev.key.keysym.sym)
                {
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        menu.select();
                        break;

                    case SDLK_ESCAPE:
                        running = false;
                        break;

                    case SDLK_LEFT:
                    case SDLK_UP:
                        menu.prev();
                        break;

                    case SDLK_RIGHT:
                    case SDLK_DOWN:
                        menu.next();
                        break;

                    default:
                        break;
                }
                break;

            case SDL_JOYBUTTONDOWN: // all joystick buttons launch the selected app
                if(!joysticks.at(ev.jbutton.which).is_gc())
                {
                    std::cout<<"Joybutton: "<<ev.jbutton.which<<' '<<(int)ev.jbutton.button<<' '<<(int)ev.jbutton.state<<'\n';
                    menu.select();
                }
                break;

            case SDL_CONTROLLERBUTTONDOWN:
                if(joysticks.at(ev.jbutton.which).is_gc())
                {
                    std::cout<<"Controllerbutton: "<<ev.cbutton.which<<' '<<(int)ev.cbutton.button<<' '<<(int)ev.cbutton.state<<'\n';

                    switch(ev.cbutton.button)
                    {
                        case SDL_CONTROLLER_BUTTON_A:
                        case SDL_CONTROLLER_BUTTON_B:
                        case SDL_CONTROLLER_BUTTON_X:
                        case SDL_CONTROLLER_BUTTON_Y:
                        case SDL_CONTROLLER_BUTTON_START:
                        case SDL_CONTROLLER_BUTTON_BACK:
                        case SDL_CONTROLLER_BUTTON_GUIDE:
                            menu.select();
                            break;

                        case SDL_CONTROLLER_BUTTON_DPAD_UP:
                        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
                            menu.prev();
                            break;

                        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
                            menu.next();
                            break;

                        default:
                            break;
                    }
                }
                break;

            case SDL_JOYHATMOTION:
                if(!joysticks.at(ev.jhat.which).is_gc())
                {
                    std::cout<<"Joyhat: "<<ev.jhat.which<<' '<<(int)ev.jhat.hat<<' '<<(int)ev.jhat.value<<'\n';
                    switch(ev.jhat.value)
                    {
                        case SDL_HAT_LEFT:
                        case SDL_HAT_UP:
                        case SDL_HAT_LEFTUP:
                            menu.prev();
                            break;

                        case SDL_HAT_RIGHT:
                        case SDL_HAT_DOWN:
                        case SDL_HAT_RIGHTDOWN:
                            menu.next();
                            break;
                        default:
                            break;
                    }
                    break;
                }

            case SDL_JOYAXISMOTION:
            case SDL_CONTROLLERAXISMOTION:
            {
                auto move = joysticks.at(ev.type == SDL_JOYAXISMOTION ? ev.jaxis.which : ev.caxis.which).menu_move(ev);

                switch(move)
                {
                    case SDL::Joystick::Dir::PREV:
                        menu.prev();
                        break;
                    case SDL::Joystick::Dir::NEXT:
                        menu.next();
                        break;
                    default:
                        break;
                }

                break;
            }

            default:
                break;
        }

        std::cout.flush();
        SDL_RenderClear(renderer);

        menu.draw(renderer, tex);

        // SDL_RenderCopy(renderer, tex, nullptr, nullptr);

        SDL_RenderPresent(renderer);
    }

    return 0;
}
