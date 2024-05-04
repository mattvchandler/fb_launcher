#include <iostream>
#include <map>

#include "cec.hpp"
#include "font.hpp"
#include "joystick.hpp"
#include "sdl.hpp"
#include "texture.hpp"

// TODO: load / parse config file
// TODO: unload everything and launch selected program
// TODO: power-off, reboot options
// TODO: autolaunch 1st app (Kodi)

void menu_prev()
{
    std::cout<<"Prev menu item stub\n";
}

void menu_next()
{
    std::cout<<"Next menu item stub\n";
}

void menu_select()
{
    std::cout<<"Menu select stub\n";
}

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
    SDL_ShowCursor(SDL_DISABLE);

    auto tex = SDL::Texture(renderer, argv[1]);

    // TODO: scale to window size - will need to be reloaded if window changes size
    auto title_font = SDL::Font("sans-serif", 40);
    auto desc_font = SDL::Font("sans-serif", 20);

    auto joysticks = std::map<int, SDL::Joystick>{};

    CEC_Input cec;
    cec.register_up(menu_prev);
    cec.register_left(menu_prev);
    cec.register_down(menu_next);
    cec.register_right(menu_next);
    cec.register_select(menu_select);

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
                        menu_select();
                        break;

                    case SDLK_ESCAPE:
                        running = false;
                        break;

                    case SDLK_LEFT:
                    case SDLK_UP:
                        menu_prev();
                        break;

                    case SDLK_RIGHT:
                    case SDLK_DOWN:
                        menu_next();
                        break;

                    default:
                        break;
                }
                break;

            case SDL_JOYBUTTONDOWN: // all joystick buttons launch the selected app
                if(!joysticks.at(ev.jbutton.which).is_gc())
                {
                    std::cout<<"Joybutton: "<<ev.jbutton.which<<' '<<(int)ev.jbutton.button<<' '<<(int)ev.jbutton.state<<'\n';
                    menu_select();
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
                            menu_select();
                            break;

                        case SDL_CONTROLLER_BUTTON_DPAD_UP:
                        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
                            menu_prev();
                            break;

                        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
                            menu_next();
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
                            menu_prev();
                            break;

                        case SDL_HAT_RIGHT:
                        case SDL_HAT_DOWN:
                        case SDL_HAT_RIGHTDOWN:
                            menu_next();
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
                        menu_prev();
                        break;
                    case SDL::Joystick::Dir::NEXT:
                        menu_next();
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

        int w = 0, h = 0;
        SDL_GetRendererOutputSize(renderer, &w, &h);

        // units are percentages
        constexpr auto row_height = 20;
        constexpr auto horiz_margin = 10;
        constexpr auto row_spacing = 10;
        constexpr auto col_spacing = 5;

        const auto row_height_px = row_height * h / 100;
        const auto horiz_margin_px = horiz_margin * w / 100;
        const auto row_spacing_px = row_spacing * h / 100;
        const auto col_spacing_px = col_spacing * w / 100;

        const auto image_size_px = row_height_px;
        const auto row_top_px = (h - row_height_px) / 2;
        const auto text_x = horiz_margin_px + image_size_px + col_spacing_px;

        // auto text = font.render_text(renderer, "Testing this\nText", SDL_Color{0xFF, 0xFF, 0xFF, 0xFF});
        auto title_text = title_font.render_text(renderer, "Menu item # 1", {0xFF, 0xFF, 0xFF, 0xFF});
        auto desc_text = desc_font.render_text(renderer, "This is the description for my menu item. It is rather long and ought to wrap so I'm going to keep typing to make sure that it does and this is probably enough text that it will wrap so I'm going to stop now.",
                {0xFF, 0xFF, 0xFF, 0xFF}, w - (2 * horiz_margin_px + image_size_px + col_spacing_px));

        tex.render(renderer, horiz_margin_px, row_top_px, image_size_px, image_size_px);
        title_text.render(renderer, text_x, row_top_px);
        desc_text.render(renderer, text_x, row_top_px + title_text.get_height());

        // TODO fade upper & lower rows. Also make a row function
        tex.render(renderer, horiz_margin_px, row_top_px - (row_height_px + row_spacing_px), image_size_px, image_size_px);
        tex.render(renderer, horiz_margin_px, row_top_px + (row_height_px + row_spacing_px), image_size_px, image_size_px);

        // TODO: BG image (blended with black?)
        // SDL_RenderCopy(renderer, tex, nullptr, nullptr);

        SDL_RenderPresent(renderer);
    }

    return 0;
}
