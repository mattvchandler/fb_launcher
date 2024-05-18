#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <span>
#include <variant>

#include "sdl.hpp"

namespace SDL
{
    class Texture
    {
    private:
        SDL_Texture * texture_ {nullptr};
        int width_ {0};
        int height_ {0};

        std::variant<std::string, std::span<char>> stored_image_;
        bool rescalable_ {false};

    public:
        Texture() = default;
        Texture(Renderer & renderer, int width, int height):
            texture_{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height)},
            width_{width}, height_{height}
        {
            if(!texture_)
                sdl_error("Unable to create SDL texture");
        }
        Texture(Renderer & renderer, const std::string & img_path,
                int viewport_width = 0, int viewport_height = 0);

        Texture(Renderer & renderer, const std::span<char> & img_data,
                int viewport_width = 0, int viewport_height = 0);

        Texture(Renderer & renderer, Surface & surface):
            texture_{SDL_CreateTextureFromSurface(renderer, surface)},
            width_{surface->w}, height_{surface->h}
        {
            if(!texture_)
                sdl_error("Unable to create SDL texture");
        }
        ~Texture()
        {
            if(texture_)
                SDL_DestroyTexture(texture_);
        }

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;

        Texture(Texture && t):
            texture_{t.texture_},
            width_{std::move(t.width_)},
            height_{std::move(t.height_)},
            stored_image_{std::move(t.stored_image_)},
            rescalable_{std::move(t.rescalable_)}
        {
            t.texture_ = nullptr;
        }
        Texture &operator=(Texture && t)
        {
            if(&t != this)
            {
                texture_ = t.texture_;
                t.texture_ = nullptr;
                width_ = std::move(t.width_);
                height_ = std::move(t.height_);
                stored_image_ = std::move(t.stored_image_);
                rescalable_ = std::move(t.rescalable_);
            }
            return *this;
        }

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

        void rescale(Renderer & renderer, int width, int height);
    };
}
#endif // TEXTURE_HPP
