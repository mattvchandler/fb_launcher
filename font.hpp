#ifndef FONT_HPP
#define FONT_HPP

#include <string>
#include <stdexcept>

#include <SDL2/SDL_ttf.h>

#include "sdl.hpp"

namespace
{
    [[noreturn]] inline void ttf_error(const std::string & error)
    {
        throw std::runtime_error{error + ": " + TTF_GetError()};
    }
}
namespace SDL
{
    struct TTF
    {
        TTF()
        {
            if(TTF_Init() < 0)
                ttf_error("Unable to initialize TTF");
        }
        ~TTF() { TTF_Quit(); }
    };

    struct Font
    {
        TTF_Font * font {nullptr};
        explicit Font(const std::string font_name = "sans-serif", int ptsize = 12);
        ~Font() { TTF_CloseFont(font); }
        operator const TTF_Font*() const { return font; }
        operator TTF_Font*() { return font; }

        Texture render_text(Renderer & renderer, const std::string & text, SDL_Color color, int wrap_length = 0);
    };
}

#endif // FONT_HPP
