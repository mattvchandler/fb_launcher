#ifndef SDL_HPP
#define SDL_HPP

#include <string>
#include <stdexcept>

#include <SDL2/SDL.h>

namespace
{
    [[noreturn]] inline void sdl_error(const std::string & error)
    {
        throw std::runtime_error{error + ": " + SDL_GetError()};
    }
}

namespace SDL
{
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
            renderer{SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)}
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

    struct Texture
    {
        SDL_Texture * texture {nullptr};
        Texture(Renderer & renderer, int width, int height):
            texture{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height)}
        {
            if(!texture)
                sdl_error("Unable to create SDL texture");
        }
        Texture(Renderer & renderer, const std::string & png_path);

        Texture(Renderer & renderer, Surface & surface):
            texture{SDL_CreateTextureFromSurface(renderer, surface)}
        {
            if(!texture)
                sdl_error("Unable to create SDL texture");
        }
        ~Texture() { SDL_DestroyTexture(texture); }
        operator const SDL_Texture*() const { return texture; }
        operator SDL_Texture*() { return texture; }

        std::pair<int, int> get_dims() const
        {
            std::pair<int, int> dims;
            if(SDL_QueryTexture(texture, nullptr, nullptr, &dims.first, &dims.second) < 0)
                sdl_error("Could not get texture dimensions");
            return dims;
        }

        int get_width() const { return get_dims().first; }
        int get_height() const { return get_dims().second; }

    };
}

#endif // SDL_HPP
