#include "texture.hpp"
#include "sdl.hpp"

#include <array>
#include <fstream>
#include <stdexcept>
#include <tuple>
#include <variant>
#include <vector>

#include <cstring>
#include <cerrno>

#include <png.h>
#include <librsvg/rsvg.h>

namespace
{
    struct RAII_stack
    {
        ~RAII_stack()
        {
            for(auto i = std::rbegin(objs); i != std::rend(objs); ++i)
            {
                auto [d, free_fun] = *i;
                free_fun(d);
            }
        }
        template <typename T, typename U>
        void push(T * d, void (*free_fun)(U*))
        {
            objs.emplace_back(reinterpret_cast<void*>(d), reinterpret_cast<void (*)(void*)>(free_fun));
        }

        std::vector<std::pair<void *, void (*)(void*)>> objs;
    };

    std::vector<char> read_to_vector(const std::string & path)
    {
        std::vector<char> data;
        std::array<char, 4096> buffer;

        auto input = std::ifstream{path, std::ios::binary};
        if(!input)
            throw std::runtime_error {"Error opening input file: " + path + " - " + strerror(errno)};
        while(input)
        {
            input.read(std::data(buffer), std::size(buffer));
            if(input.bad())
                throw std::runtime_error {"Error reading input file: " + path + " - " + strerror(errno)};

            data.insert(std::end(data), std::begin(buffer), std::begin(buffer) + input.gcount());
        }

        return data;
    }

    struct not_svg_error: public std::runtime_error
    {
        not_svg_error(const std::string & what): std::runtime_error(what) {}
    };

    std::tuple<std::vector<unsigned char>, int, int> read_png(const std::span<char> & png_mem,
            int viewport_width, int viewport_height)
    {
        RAII_stack rs;
        png_image png;
        rs.push(&png, png_image_free);

        std::memset(&png, 0, sizeof(png));
        png.version = PNG_IMAGE_VERSION;

        if(!png_image_begin_read_from_memory(&png, std::data(png_mem), std::size(png_mem)))
        {
            throw std::runtime_error{"Unable to open PNG: " + std::string{png.message}};
        }

        png.format = PNG_FORMAT_RGBA;

        std::vector<unsigned char> raw_pixel_data(PNG_IMAGE_SIZE(png));

        if(!png_image_finish_read(&png, nullptr, std::data(raw_pixel_data), PNG_IMAGE_ROW_STRIDE(png), nullptr))
        {
            throw std::runtime_error{"Unable to read PNG: " + std::string{png.message}};
        }

        int x_offset = 0, y_offset = 0;

        if(viewport_width > 0 && viewport_height > 0)
        {
            auto img_ratio = static_cast<float>(png.width) / png.height;
            auto viewport_ratio = static_cast<float>(viewport_width) / viewport_height;

            if(viewport_ratio > img_ratio)
            {
                viewport_width = static_cast<int>(png.height * viewport_ratio);
                viewport_height = png.height;
                x_offset = (viewport_width - png.width) / 2;
            }
            else
            {
                viewport_width = png.width;
                viewport_height = static_cast<int>(png.width / viewport_ratio);
                y_offset = (viewport_height - png.height) / 2;
            }
        }
        else
        {
            viewport_width = png.width;
            viewport_height = png.height;
        }

        std::vector<unsigned char> letterboxed_pixel_data(viewport_width * viewport_height * 4, 0);

        for(auto row = 0u; row < png.height; ++row)
            std::memcpy(std::data(letterboxed_pixel_data) + ((y_offset + row) * viewport_width + x_offset) * 4, std::data(raw_pixel_data) + row * png.width * 4,  png.width * 4);

        return {letterboxed_pixel_data, viewport_width, viewport_height};
    }

    std::tuple<std::vector<unsigned char>, int, int> read_svg(const std::span<char> & svg_data,
            int viewport_width, int viewport_height)
    {
        RAII_stack rs;

        GFile * file = g_file_new_for_path(".");
        rs.push(file, g_object_unref);

        GInputStream * is = g_memory_input_stream_new_from_data(std::data(svg_data), std::size(svg_data), nullptr);
        rs.push(is, g_object_unref);

        GError * err {nullptr};
        RsvgHandle * handle = rsvg_handle_new_from_stream_sync(is, file, RSVG_HANDLE_FLAGS_NONE, nullptr, &err);
        if(!handle)
        {
            rs.push(err, g_error_free);
            throw not_svg_error{std::string{err->message}};
        }
        rs.push(handle, g_object_unref);
        rsvg_handle_set_dpi(handle, 96.0);

        double width{0}, height{0};
    #if LIBRSVG_MAJOR_VERSION > 2 || (LIBRSVG_MAJOR_VERSION == 2 && LIBRSVG_MINOR_VERSION >= 52)
        rsvg_handle_get_intrinsic_size_in_pixels(handle, &width, &height);
        if(width == 0.0 || height == 0.0)
        {
            gboolean has_viewbox;
            RsvgRectangle viewbox;
            rsvg_handle_get_intrinsic_dimensions(handle, nullptr, nullptr, nullptr, nullptr, &has_viewbox, &viewbox);
            if(has_viewbox)
            {
                width = viewbox.width;
                height = viewbox.height;
            }
        }
    #else
        RsvgDimensionData dims;
        rsvg_handle_get_dimensions(svg_handle, &dims);
        width = dims.width;
        height = dims.height;
    #endif

        double x_offset = 0, y_offset = 0;

        if(viewport_width > 0 && viewport_height > 0)
        {
            auto img_ratio = width / height;
            auto viewport_ratio = static_cast<double>(viewport_width) / viewport_height;

            if(viewport_ratio > img_ratio)
            {
                width = viewport_height * img_ratio;
                height = viewport_height;
                x_offset = (viewport_width - width) / 2.0;
            }
            else
            {
                width = viewport_width;
                height = viewport_width / img_ratio;
                y_offset = (viewport_height - height) / 2.0;
            }
        }

        int pixel_width = static_cast<int>(width + 2.0 * x_offset);
        int pixel_height = static_cast<int>(height + 2.0 * y_offset);

        cairo_surface_t * surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, pixel_width, pixel_height);
        rs.push(surface, cairo_surface_destroy);
        if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
            throw std::runtime_error { "Error creating SVG cairo surface" };

        cairo_t * cr = cairo_create(surface);
        rs.push(cr, cairo_destroy);
        if(cairo_status(cr) != CAIRO_STATUS_SUCCESS)
            throw std::runtime_error {"Error creating SVG cairo object"};

    #if LIBRSVG_MAJOR_VERSION > 2 || (LIBRSVG_MAJOR_VERSION == 2 && LIBRSVG_MINOR_VERSION >= 52)
        auto viewport = RsvgRectangle {.x=x_offset, .y=y_offset, .width=width, .height=height};
        if(GError * err = nullptr; !rsvg_handle_render_document(handle, cr, &viewport, &err))
        {
            rs.push(err, g_error_free);
            throw std::runtime_error{"Error rendering SVG: " + std::string{err->message}};
        }
    #else
        if(!rsvg_handle_render_cairo(handle, cr))
        {
            throw std::runtime_error{"Error rendering SVG"};
        }
    #endif

        std::vector<unsigned char> letterboxed_pixel_data((2 * y_offset + height) * (2 * x_offset + width) * 4, 0);

        if(cairo_image_surface_get_stride(surface) < pixel_width * 4)
            throw std::runtime_error {"Invalid SVG stride"};

        auto * surface_data = cairo_image_surface_get_data(surface);
        for(int row = 0; row < pixel_height; ++row)
            std::memcpy(std::data(letterboxed_pixel_data) + row * pixel_width * 4, surface_data + cairo_image_surface_get_stride(surface) * row, pixel_width * 4);

        return {letterboxed_pixel_data, pixel_width, pixel_height};
    }

    std::tuple<std::vector<unsigned char>, int, int, bool> load_image_from_span(const std::span<char> & image_data,
            int viewport_height, int viewport_width)
    {
        const std::array<unsigned char, 8> png_header = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
        auto is_png = std::equal(std::begin(png_header), std::end(png_header), std::begin(image_data), [](unsigned char a, char b) { return a == static_cast<unsigned char>(b); });

        if(is_png)
        {
            auto && [data, width, height] = read_png(image_data, viewport_width, viewport_height);
            return {data, width, height, false};
        }
        else
        {
            try
            {
                auto && [data, width, height] = read_svg(image_data, viewport_width, viewport_height);
                return {data, width, height, true};
            }
            catch(const not_svg_error & e)
            {
                throw std::runtime_error {"Image type not supported"};
            }
        }
    }

    SDL_Texture * load_texture_from_data(SDL::Renderer & renderer, const unsigned char * raw_pixel_data, int width, int height)
    {
        auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
        if(!texture)
            SDL::sdl_error("Unable to create SDL texture");

        if(SDL_UpdateTexture(texture, nullptr, raw_pixel_data, 4 * width) < 0)
        {
            SDL_DestroyTexture(texture);
            SDL::sdl_error("Unable to load SDL texture");
        }

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(texture, SDL_ScaleModeBest);

        return texture;
    }
}

namespace SDL
{
    Texture::Texture(Renderer & renderer, const std::string & img_path,
            int viewport_width, int viewport_height):
        stored_image_{img_path}
    {
        auto file_data = read_to_vector(img_path);
        auto image_data = load_image_from_span(std::span{std::data(file_data), std::size(file_data)}, viewport_width, viewport_height);
        std::tie(std::ignore, width_, height_, rescalable_) = image_data;
        texture_ = load_texture_from_data(renderer, std::data(std::get<0>(image_data)), width_, height_);
    }

    Texture::Texture(Renderer & renderer, const std::span<char> & img_data,
            int viewport_width, int viewport_height):
        stored_image_{img_data}
    {
        auto image_data = load_image_from_span(img_data, viewport_width, viewport_height);
        std::tie(std::ignore, width_, height_, rescalable_) = image_data;
        texture_ = load_texture_from_data(renderer, std::data(std::get<0>(image_data)), width_, height_);
    }

    void Texture::render(Renderer & renderer, int x, int y, int size_w, int size_h)
    {
        if(!texture_)
            return;

        SDL_Rect render_dest;

        render_dest.x = x;
        render_dest.y = y;
        render_dest.w = size_w ? size_w : width_;
        render_dest.h = size_h ? size_h : height_;
        SDL_RenderCopy(renderer, texture_, nullptr, &render_dest);
    }

    void Texture::rescale(Renderer & renderer, int width, int height)
    {
        if(!texture_ || !rescalable_)
            return;

        if(auto filename = std::get_if<std::string>(&stored_image_); filename)
            *this = Texture{renderer, *filename, width, height};
        else
            *this = Texture{renderer, std::get<std::span<char>>(stored_image_), width, height};
    }
}
