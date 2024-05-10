#include "menu.hpp"

#include <functional>
#include <iostream> // TODO: delete
#include <vector>

namespace
{
    // units are percentages
    constexpr auto row_height = 20;
    constexpr auto horiz_margin = 10;
    constexpr auto row_spacing = 10;
    constexpr auto col_spacing = 5;

    constexpr auto text_color = SDL_Color {0xFF, 0xFF, 0xFF, 0xFF};
}

Menu::Menu(const std::vector<App> & apps):
    apps_{apps}
{
    SDL_ShowCursor(SDL_DISABLE);

    cec_.register_up(std::bind(&Menu::prev, this));
    cec_.register_left(std::bind(&Menu::prev, this));
    cec_.register_down(std::bind(&Menu::next, this));
    cec_.register_right(std::bind(&Menu::next, this));
    cec_.register_select(std::bind(&Menu::select, this));

    for(auto && a: apps_)
    {
        // Note: text textures are generated in resize()
        app_textures_.emplace_back(Menu_textures{
            .thumbnail = a.thumbnail_path.empty() ? SDL::Texture{} : SDL::Texture{renderer_, a.thumbnail_path},
            .bg        = a.bg_path.empty()        ? SDL::Texture{} : SDL::Texture{renderer_, a.bg_path}});
    }
}

int Menu::run()
{
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
                    SDL_GetRendererOutputSize(renderer_, &w, &h);

                    std::cout<<"Window resize event: "<<w<<' '<<h<<'\n';

                    resize(w, h);
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
                        select();
                        break;

                    case SDLK_ESCAPE:
                        running = false;
                        break;

                    case SDLK_LEFT:
                    case SDLK_UP:
                        prev();
                        break;

                    case SDLK_RIGHT:
                    case SDLK_DOWN:
                        next();
                        break;

                    default:
                        break;
                }
                break;

            case SDL_JOYBUTTONDOWN: // all joystick buttons launch the selected app
                if(!joysticks.at(ev.jbutton.which).is_gc())
                {
                    std::cout<<"Joybutton: "<<ev.jbutton.which<<' '<<(int)ev.jbutton.button<<' '<<(int)ev.jbutton.state<<'\n';
                    select();
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
                            select();
                            break;

                        case SDL_CONTROLLER_BUTTON_DPAD_UP:
                        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
                            prev();
                            break;

                        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
                            next();
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
                            prev();
                            break;

                        case SDL_HAT_RIGHT:
                        case SDL_HAT_DOWN:
                        case SDL_HAT_RIGHTDOWN:
                            next();
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
                        prev();
                        break;
                    case SDL::Joystick::Dir::NEXT:
                        next();
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
        SDL_RenderClear(renderer_);

        draw();

        // SDL_RenderCopy(renderer_, tex, nullptr, nullptr);

        SDL_RenderPresent(renderer_);
    }

    return 0; // TODO: return selected
}

// TODO: animate scrolling
void Menu::prev()
{
    index_ = index_ == 0 ? std::size(apps_) - 1 : index_ - 1;
}

void Menu::next()
{
    index_ = index_ == std::size(apps_) - 1 ? 0 : index_ + 1;
}

void Menu::select()
{
    std::cout<<"Menu select stub\n";
    // TODO: set selection & exit
}

void Menu::resize(int w, int h)
{
    if(w != w_ || h != h_)
    {
        w_ = w; h_ = h;

        const auto font_size = h / 20;

        // TODO: don't repeat
        const auto row_height_px = row_height * h_ / 100;
        const auto horiz_margin_px = horiz_margin * w_ / 100;
        const auto col_spacing_px = col_spacing * w_ / 100;
        const auto image_size_px = row_height_px;

        const auto text_wrap_px = w_ - (2 * horiz_margin_px + image_size_px + col_spacing_px);

        std::cout<<"Font sizes: "<<font_size<<" "<<font_size/2<<'\n';

        auto title_font = SDL::Font{"sans-serif", font_size};
        auto desc_font = SDL::Font{"sans-serif", font_size / 2};

        for(auto i = 0u; i < std::size(apps_); ++i)
        {
            app_textures_[i].title = title_font.render_text(renderer_, apps_[i].title, text_color, text_wrap_px);
            app_textures_[i].desc = desc_font.render_text(renderer_, apps_[i].desc, text_color, text_wrap_px);
        }
    }
}

void Menu::draw()
{
    if(w_ == 0 || h_ == 0)
        return;

    // TODO: BG image (blended with black?)
    draw_row(-1);
    draw_row(0);
    draw_row(1);
}

void Menu::draw_row(int pos)
{
    const auto row_height_px = row_height * h_ / 100;
    const auto horiz_margin_px = horiz_margin * w_ / 100;
    const auto row_spacing_px = row_spacing * h_ / 100;
    const auto col_spacing_px = col_spacing * w_ / 100;

    const auto image_size_px = row_height_px;
    const auto row_top_px = h_ / 2 + pos * (row_height_px + row_spacing_px) - row_height_px / 2;
    const auto text_x = horiz_margin_px + image_size_px + col_spacing_px;

    auto row_index = static_cast<int>(index_) + pos;

    if(row_index >= static_cast<int>(std::size(apps_)))
        row_index %= std::size(apps_);
    while(row_index < 0)
        row_index += std::size(apps_);

    auto & tex = app_textures_[row_index];

    const auto fade = pos == 0 ? 255 : 64;
    SDL_SetTextureColorMod(tex.thumbnail, fade, fade, fade);
    SDL_SetTextureColorMod(tex.title, fade, fade, fade);
    SDL_SetTextureColorMod(tex.desc, fade, fade, fade);

    tex.thumbnail.render(renderer_, horiz_margin_px, row_top_px, image_size_px, image_size_px);
    tex.title.render(renderer_, text_x, row_top_px);
    tex.desc.render(renderer_, text_x, row_top_px + tex.title.get_height());
}
