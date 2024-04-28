#ifndef SDL_HPP
#define SDL_HPP

#include <algorithm>
#include <string>
#include <stdexcept>
#include <variant>
#include <vector>

#include <cmath>

#include <SDL2/SDL.h>

namespace SDL
{
    [[noreturn]] inline void sdl_error(const std::string & error)
    {
        throw std::runtime_error{error + ": " + SDL_GetError()};
    }

    struct SDL
    {
        SDL(Uint32 flags)
        {
            if(SDL_Init(flags) < 0)
                sdl_error("Unable to initialize SDL");
        }
        ~SDL() { SDL_Quit(); }
    };

    struct Window
    {
        SDL_Window * window {nullptr};
        explicit Window(const char * title,
            int x = SDL_WINDOWPOS_UNDEFINED,
            int y = SDL_WINDOWPOS_UNDEFINED,
            int w = 800,
            int h = 600,
            Uint32 flags = SDL_WINDOW_FULLSCREEN_DESKTOP):
            window{SDL_CreateWindow(title, x, y, w, h, flags)}
        {
            if(!window)
                sdl_error("Unable to create SDL window");
        }
        ~Window() { SDL_DestroyWindow(window); }
        operator const SDL_Window*() const { return window; }
        operator SDL_Window*() { return window; }
    };

    struct Renderer
    {
        SDL_Renderer * renderer {nullptr};
        explicit Renderer(Window & window):
            renderer{SDL_CreateRenderer(window, -1, 0)}
        {
            if(!renderer)
                sdl_error("Unable to create SDL renderer");
        }
        ~Renderer() { SDL_DestroyRenderer(renderer); }
        operator const SDL_Renderer*() const { return renderer; }
        operator SDL_Renderer*() { return renderer; }
    };

    struct Surface
    {
        SDL_Surface * surface {nullptr};
        explicit Surface(SDL_Surface * s): surface{s} {}
        ~Surface() { if(surface) SDL_FreeSurface(surface); }
        operator const SDL_Surface*() const { return surface; }
        operator SDL_Surface*() { return surface; }
        const SDL_Surface * operator->() const {return surface; }
        SDL_Surface * operator->() {return surface; }
    };

    class Texture
    {
    private:
        SDL_Texture * texture_ {nullptr};
        int width_ {0};
        int height_ {0};

        SDL_Rect render_dest_;

    public:
        Texture(Renderer & renderer, int width, int height):
            texture_{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height)},
            width_{width}, height_{height}
        {
            if(!texture_)
                sdl_error("Unable to create SDL texture");
        }
        Texture(Renderer & renderer, const std::string & png_path);

        Texture(Renderer & renderer, Surface & surface):
            texture_{SDL_CreateTextureFromSurface(renderer, surface)},
            width_{surface->w}, height_{surface->h}
        {
            if(!texture_)
                sdl_error("Unable to create SDL texture");
        }
        ~Texture() { SDL_DestroyTexture(texture_); }
        operator const SDL_Texture*() const { return texture_; }
        operator SDL_Texture*() { return texture_; }

        std::pair<int, int> get_dims() const
        {
            std::pair<int, int> dims;
            if(SDL_QueryTexture(texture_, nullptr, nullptr, &dims.first, &dims.second) < 0)
                sdl_error("Could not get texture dimensions");
            return dims;
        }

        void render(Renderer & renderer, int x, int y, int size_w = 0, int size_h = 0);

        int get_width() const { return width_; }
        int get_height() const { return height_; }
    };

    class Joystick
    {
    private:
        static constexpr auto deadzone = 8'000;
        static constexpr auto threshold = 20'000;

        std::variant<SDL_Joystick*, SDL_GameController*> joystick {static_cast<SDL_Joystick*>(nullptr)};
        std::vector<char> centered;
    public:

        enum class Dir {NONE, PREV, NEXT};

        explicit Joystick(int index) : joystick{SDL_JoystickOpen(index)}
        {
            if(SDL_IsGameController(index))
            {
                auto g = SDL_GameControllerOpen(index);
                if(!g)
                    sdl_error("Unable to create SDL gamecontroller");
                joystick = g;

                centered.resize(SDL_CONTROLLER_AXIS_MAX);
                std::fill(std::begin(centered), std::end(centered), 1);
            }
            else
            {
                auto j = SDL_JoystickOpen(index);
                if(!j)
                    sdl_error("Unable to create SDL joystick");

                centered.resize(SDL_JoystickNumAxes(j));
                std::fill(std::begin(centered), std::end(centered), 1);
                joystick = j;
            }
        }

        Dir menu_move(const SDL_Event & ev)
        {
            char * ctr = nullptr;
            Sint16 x = 0, y = 0;

            if(ev.type == SDL_JOYAXISMOTION && std::holds_alternative<SDL_Joystick* >(joystick))
            {
                x = ev.jaxis.value;
                ctr = &centered[ev.jaxis.axis];
            }
            else if(ev.type == SDL_CONTROLLERAXISMOTION && std::holds_alternative<SDL_GameController*>(joystick))
            {
                // We're tracking centering for both axes in the centered[...X] var
                auto & c = std::get<SDL_GameController*>(joystick);
                switch(ev.caxis.axis)
                {
                    case SDL_CONTROLLER_AXIS_LEFTX:
                    case SDL_CONTROLLER_AXIS_LEFTY:
                        ctr = &centered[SDL_CONTROLLER_AXIS_LEFTX];
                        x = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTX);
                        y = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTY);
                        break;
                    case SDL_CONTROLLER_AXIS_RIGHTX:
                    case SDL_CONTROLLER_AXIS_RIGHTY:
                        ctr = &centered[SDL_CONTROLLER_AXIS_RIGHTX];
                        x = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_RIGHTX);
                        y = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_RIGHTY);
                        break;
                    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                        ctr = &centered[ev.caxis.axis];
                        x = ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT ? ev.caxis.value : -ev.caxis.value;
                        break;
                }
            }
            else
                return Dir::NONE;

            auto len2 = x*x + y*y;
            constexpr auto deadzone2 = deadzone*deadzone;
            constexpr auto threshold2 = threshold*threshold;

            if(len2 > threshold2 && *ctr)
            {
                *ctr = false;
                if(std::abs(x) > std::abs(y))
                    return x < 0 ? Dir::PREV : Dir::NEXT;
                else
                    return y < 0 ? Dir::PREV : Dir::NEXT;
            }
            else if(len2 < deadzone2)
            {
                *ctr = true;
                return Dir::NONE;
            }
            else
                return Dir::NONE;
        }

        const char * name()
        {
            if(std::holds_alternative<SDL_Joystick*>(joystick))
                return SDL_JoystickName(std::get<SDL_Joystick*>(joystick));
            else
                return SDL_GameControllerName(std::get<SDL_GameController*>(joystick));
        }

        Joystick(const Joystick &) = delete;
        Joystick &operator=(const Joystick &) = delete;
        Joystick(Joystick && j):
            joystick{j.joystick},
            centered{std::move(j.centered)}
        {
            j.joystick = static_cast<SDL_Joystick*>(nullptr);
        }
        Joystick &operator=(Joystick && j)
        {
            if(this != &j)
            {
                joystick = j.joystick;
                j.joystick = static_cast<SDL_Joystick*>(nullptr);
                centered = std::move(j.centered);
            }
            return *this;
        }
        ~Joystick()
        {
            if(std::holds_alternative<SDL_Joystick*>(joystick))
            {
                auto & j = std::get<SDL_Joystick*>(joystick);
                if(j)
                    SDL_JoystickClose(j);
            }
            else
            {
                auto & g = std::get<SDL_GameController*>(joystick);
                if(g)
                    SDL_GameControllerClose(g);
            }
        }
    };
}

#endif // SDL_HPP
