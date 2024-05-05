#ifndef SDL_HPP
#define SDL_HPP

#include <string>
#include <stdexcept>

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

        Window(const Window &) = delete;
        Window(Window &&) = delete;
        Window &operator=(const Window &) = delete;
        Window &operator=(Window &&) = delete;

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

        Renderer(const Renderer &) = delete;
        Renderer(Renderer &&) = delete;
        Renderer &operator=(const Renderer &) = delete;
        Renderer &operator=(Renderer &&) = delete;

        operator const SDL_Renderer*() const { return renderer; }
        operator SDL_Renderer*() { return renderer; }
    };

    struct Surface
    {
        SDL_Surface * surface {nullptr};
        explicit Surface(SDL_Surface * s): surface{s} {}
        ~Surface() { if(surface) SDL_FreeSurface(surface); }
        Surface(const Surface &) = delete;
        Surface(Surface &&) = delete;
        Surface &operator=(const Surface &) = delete;
        Surface &operator=(Surface &&) = delete;
        operator const SDL_Surface*() const { return surface; }
        operator SDL_Surface*() { return surface; }
        const SDL_Surface * operator->() const {return surface; }
        SDL_Surface * operator->() {return surface; }
    };

}

#endif // SDL_HPP
