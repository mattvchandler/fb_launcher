#include "texture.hpp"

#include <vector>
#include <cstring>

#include <png.h>

namespace SDL
{
    Texture::Texture(Renderer & renderer, const std::string & png_path)
    {
        png_image png;
        std::memset(&png, 0, sizeof(png));
        png.version = PNG_IMAGE_VERSION;

        if(!png_image_begin_read_from_file(&png, png_path.c_str()))
            throw std::runtime_error{"Unable to open PNG: " + std::string{png.message}};

        png.format = PNG_FORMAT_RGBA;

        std::vector<unsigned char> raw_pixel_data(PNG_IMAGE_SIZE(png));

        if(!png_image_finish_read(&png, nullptr, std::data(raw_pixel_data), PNG_IMAGE_ROW_STRIDE(png), nullptr))
            throw std::runtime_error{"Unable to read PNG: " + std::string{png.message}};

        width_ = png.width;
        height_ = png.height;

        texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width_, height_);
        if(!texture_)
            sdl_error("Unable to create SDL texture");

        if(SDL_UpdateTexture(texture_, nullptr, std::data(raw_pixel_data), 4 * png.width) < 0)
            sdl_error("Unable to load SDL texture");

        png_image_free(&png);
    }

    void Texture::render(Renderer & renderer, int x, int y, int size_w, int size_h)
    {
        render_dest_.x = x;
        render_dest_.y = y;
        render_dest_.w = size_w ? size_w : width_;
        render_dest_.h = size_h ? size_h : height_;
        SDL_RenderCopy(renderer, texture_, nullptr, &render_dest_);
    }
}
