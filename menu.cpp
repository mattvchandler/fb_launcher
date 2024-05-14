#include "menu.hpp"

#include <chrono>
#include <functional>
#include <thread>
#include <vector>

namespace
{
    struct Layout
    {
        // units are percentages
        static constexpr int row_height = 20;
        static constexpr int horiz_margin = 10;
        static constexpr int row_spacing = 10;
        static constexpr int col_spacing = 5;

        int screen_width {0};
        int screen_height {0};
        Layout(int w, int h): screen_width{w}, screen_height{h} {}

        int row_height_px() { return row_height * screen_height / 100; }
        int horiz_margin_px() { return horiz_margin * screen_width / 100; }
        int row_spacing_px() { return row_spacing * screen_height / 100; }
        int col_spacing_px() { return col_spacing * screen_width / 100; }
        int image_size_px() { return row_height_px(); }
        int text_wrap_px() { return  screen_width - (2 * horiz_margin_px() + 2 * image_size_px() + 2 * col_spacing_px()); }
        int text_x_px() { return horiz_margin_px() + image_size_px() + col_spacing_px(); }
        int input_icon_size_px() { return image_size_px() / 2; }
        int left_input_icon_x_px()  { return screen_width - horiz_margin_px() - input_icon_size_px(); }
        int right_input_icon_x_px() { return left_input_icon_x_px() + input_icon_size_px(); }
        int upper_input_icon_y_px() { return 0; }
        int lower_input_icon_y_px() { return input_icon_size_px(); }
    };

    constexpr auto text_color = SDL_Color {0xFF, 0xFF, 0xFF, 0xFF};

    constexpr auto animation_duration = std::chrono::milliseconds{200};

    constexpr auto framerate = 60.0f;
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
            .thumbnail = a.thumbnail_path.empty() ? SDL::Texture{} : SDL::Texture{renderer_, a.thumbnail_path}
        });
    }

    animation_event_ = SDL_RegisterEvents(1);
    if(animation_event_ == (Uint32)-1)
        SDL::sdl_error("Could not register custom event");
}

int Menu::run()
{
    exited_ = false;
    running_ = true;

    while(running_)
    {
        auto frame_start = std::chrono::system_clock::now();

        SDL_Event ev;
        if(SDL_WaitEvent(&ev) < 0) // might need to change this to SDL_WaitEventTimeout so CEC inputs can come through - experiment to see if that's the case
            SDL::sdl_error("Error getting SDL event");

        switch(ev.type)
        {
            case SDL_QUIT:
                running_ = false;
                exited_ = true;
                break;

            case SDL_WINDOWEVENT:
                if(ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    int w, h;
                    SDL_GetRendererOutputSize(renderer_, &w, &h);

                    resize(w, h);
                }
                break;

            case SDL_JOYDEVICEADDED:
            {
                auto joy = SDL::Joystick{ev.jdevice.which};
                joysticks.emplace(SDL_JoystickGetDeviceInstanceID(ev.jdevice.which), std::move(joy));
                break;
            }

            case SDL_JOYDEVICEREMOVED:
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
                        running_ = false;
                        exited_ = true;
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
                    select();
                break;

            case SDL_CONTROLLERBUTTONDOWN:
                if(joysticks.at(ev.jbutton.which).is_gc())
                {
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

        SDL_RenderClear(renderer_);
        draw();
        SDL_RenderPresent(renderer_);

        if(animation_direction_ != 0)
        {
            SDL_Event ev;
            SDL_zero(ev);
            ev.type = animation_event_;
            SDL_PushEvent(&ev);
        }

        auto now = std::chrono::system_clock::now();
        auto frame_time = now - frame_start;
        std::this_thread::sleep_for(std::chrono::duration<float>(1.0f / framerate) - frame_time);
    }

    return index_;
}

void Menu::prev()
{
    if(animation_direction_ == 0)
    {
        index_ = index_ == 0 ? static_cast<int>(std::size(apps_)) - 1 : index_ - 1;
        animation_start_ = std::chrono::system_clock::now();
        animation_direction_ = -1;
    }
}

void Menu::next()
{
    if(animation_direction_ == 0)
    {
        index_ = index_ == static_cast<int>(std::size(apps_)) - 1 ? 0 : index_ + 1;
        animation_start_ = std::chrono::system_clock::now();
        animation_direction_ = 1;
    }
}

void Menu::select()
{
    running_ = false;
}

void Menu::resize(int w, int h)
{
    if(w != w_ || h != h_)
    {
        w_ = w; h_ = h;

        const auto font_size = h_ / 20;

        auto title_font = SDL::Font{"sans-serif", font_size};
        auto desc_font = SDL::Font{"sans-serif", font_size / 2};

        auto layout = Layout{w_, h_};
        for(auto i = 0u; i < std::size(apps_); ++i)
        {
            app_textures_[i].title = title_font.render_text(renderer_, apps_[i].title, text_color, layout.text_wrap_px());
            app_textures_[i].desc = desc_font.render_text(renderer_, apps_[i].desc, text_color, layout.text_wrap_px());
            app_textures_[i].thumbnail.rescale(renderer_, w_, h_);
        }
    }

    mouse_icon_.rescale(renderer_, w_, h_);
    keyboard_icon_.rescale(renderer_, w_, h_);
    gamepad_icon_.rescale(renderer_, w_, h_);
    cec_icon_.rescale(renderer_, w_, h_);
}

void Menu::draw()
{
    if(w_ == 0 || h_ == 0)
        return;

    draw_row(-1);
    draw_row(0);
    draw_row(1);

    if(animation_direction_ < 0)
        draw_row(2);
    else if(animation_direction_ > 0)
        draw_row(-2);
}

void Menu::draw_row(int pos)
{
    auto animation_offset = 0;
    auto layout = Layout(w_, h_);

    if(animation_direction_ != 0)
    {
        auto animation_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - animation_start_);
        if(animation_time >= animation_duration)
        {
            animation_direction_ = 0;
        }
        else
        {
            auto percentage = static_cast<float>(animation_time.count()) / animation_duration.count();
            auto start = (layout.row_height_px() + layout.row_spacing_px()) * animation_direction_;
            animation_offset = static_cast<int>((1.0f - percentage) * start);
        }
    }

    const auto row_top_px = h_ / 2 + pos * (layout.row_height_px() + layout.row_spacing_px()) - layout.row_height_px() / 2 + animation_offset;

    auto row_index = index_ + pos;

    if(row_index >= static_cast<int>(std::size(apps_)))
        row_index %= std::size(apps_);
    while(row_index < 0)
        row_index += std::size(apps_);

    auto & tex = app_textures_[row_index];

    const auto fade = pos == 0 ? 255 : 64;
    SDL_SetTextureColorMod(tex.thumbnail, fade, fade, fade);
    SDL_SetTextureColorMod(tex.title, fade, fade, fade);
    SDL_SetTextureColorMod(tex.desc, fade, fade, fade);
    SDL_SetTextureColorMod(mouse_icon_, fade, fade, fade);
    SDL_SetTextureColorMod(keyboard_icon_, fade, fade, fade);
    SDL_SetTextureColorMod(gamepad_icon_, fade, fade, fade);
    SDL_SetTextureColorMod(cec_icon_, fade, fade, fade);

    tex.thumbnail.render(renderer_, layout.horiz_margin_px(), row_top_px, layout.image_size_px(), layout.image_size_px());
    tex.title.render(renderer_, layout.text_x_px(), row_top_px);
    tex.desc.render(renderer_, layout.text_x_px(), row_top_px + tex.title.get_height());

    auto & app = apps_[row_index];
    if(app.input_mouse)
        mouse_icon_.render(renderer_, layout.left_input_icon_x_px(), row_top_px + layout.upper_input_icon_y_px(), layout.input_icon_size_px(), layout.input_icon_size_px());
    if(app.input_keyboard)
        keyboard_icon_.render(renderer_, layout.right_input_icon_x_px(), row_top_px + layout.upper_input_icon_y_px(), layout.input_icon_size_px(), layout.input_icon_size_px());
    if(app.input_gamepad)
        gamepad_icon_.render(renderer_, layout.left_input_icon_x_px(), row_top_px + layout.lower_input_icon_y_px(), layout.input_icon_size_px(), layout.input_icon_size_px());
    if(app.input_cec)
        cec_icon_.render(renderer_, layout.right_input_icon_x_px(), row_top_px + layout.lower_input_icon_y_px(), layout.input_icon_size_px(), layout.input_icon_size_px());
}
