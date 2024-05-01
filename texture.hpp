#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "sdl.hpp"

namespace SDL
{
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
}
#endif // TEXTURE_HPP
