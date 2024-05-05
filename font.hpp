#ifndef FONT_HPP
#define FONT_HPP

#include <string>
#include <stdexcept>

#include <SDL2/SDL_ttf.h>

#include "texture.hpp"

namespace SDL
{
    [[noreturn]] inline void ttf_error(const std::string & error)
    {
        throw std::runtime_error{error + ": " + TTF_GetError()};
    }

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
        Font() = default;

        explicit Font(const std::string font_name, int ptsize);
        ~Font()
        {
            if(font)
                TTF_CloseFont(font);
        }

        Font(const Font &) = delete;
        Font &operator=(const Font &) = delete;

        Font(Font && f)
        {
            font = f.font;
            f.font = nullptr;
        }
        Font &operator=(Font && f)
        {
            if(&f != this)
            {
                font = f.font;
                f.font = nullptr;
            }
            return *this;
        }

        operator const TTF_Font*() const { return font; }
        operator TTF_Font*() { return font; }

        Texture render_text(Renderer & renderer, const std::string & text, SDL_Color color, int wrap_length = 0);
    };
}

#endif // FONT_HPP
