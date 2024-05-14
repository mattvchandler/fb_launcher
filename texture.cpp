#include "texture.hpp"

#include <vector>
#include <tuple>
#include <cstring>

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

    std::tuple<std::vector<unsigned char>, int, int> read_png(const std::string & png_path,
            int viewport_width, int viewport_height)
    {
        RAII_stack rs;
        png_image png;
        rs.push(&png, png_image_free);

        std::memset(&png, 0, sizeof(png));
        png.version = PNG_IMAGE_VERSION;

        if(!png_image_begin_read_from_file(&png, png_path.c_str()))
        {
            throw std::runtime_error{"Unable to open PNG (" + png_path + "): " + std::string{png.message}};
        }

        png.format = PNG_FORMAT_RGBA;

        std::vector<unsigned char> raw_pixel_data(PNG_IMAGE_SIZE(png));

        if(!png_image_finish_read(&png, nullptr, std::data(raw_pixel_data), PNG_IMAGE_ROW_STRIDE(png), nullptr))
        {
            throw std::runtime_error{"Unable to read PNG (" + png_path + "): " + std::string{png.message}};
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

    std::tuple<std::vector<unsigned char>, int, int> read_svg(const std::string & svg_path,
            int viewport_width, int viewport_height,
            int override_r, int override_g, int override_b)
    {
        RAII_stack rs;

        GFile * file = g_file_new_for_path(svg_path.c_str());
        rs.push(file, g_object_unref);

        GError * err {nullptr};
        RsvgHandle * handle = rsvg_handle_new_from_gfile_sync(file, RSVG_HANDLE_FLAGS_NONE, nullptr, &err);
        if(!handle)
        {
            rs.push(err, g_error_free);
            throw std::runtime_error{"Unable to open SVG (" + svg_path + "): " + std::string{err->message}};
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

        if(override_r >= 0 && override_g >= 0 && override_b >= 0)
        {
            cairo_set_source_rgba(cr, override_r / 255.0, override_g / 255.0, override_b / 255.0, 1.0);
            cairo_mask_surface(cr, surface, 0.0, 0.0);
        }

        std::vector<unsigned char> letterboxed_pixel_data((2 * y_offset + height) * (2 * x_offset + width) * 4, 0);

        if(cairo_image_surface_get_stride(surface) < pixel_width * 4)
            throw std::runtime_error {"Invalid SVG stride"};

        auto * surface_data = cairo_image_surface_get_data(surface);
        for(int row = 0; row < pixel_height; ++row)
            std::memcpy(std::data(letterboxed_pixel_data) + row * pixel_width * 4, surface_data + cairo_image_surface_get_stride(surface) * row, pixel_width * 4);

        return {letterboxed_pixel_data, pixel_width, pixel_height};
    }
}

namespace SDL
{
    Texture::Texture(Renderer & renderer, const std::string & img_path,
            int viewport_width, int viewport_height,
            int override_r, int override_g, int override_b):
        path_{img_path},
        override_r_{override_r},
        override_g_{override_g},
        override_b_{override_b}
    {
        auto raw_pixel_data = std::vector<unsigned char>{};

        if(path_.ends_with(".png"))
            std::tie(raw_pixel_data, width_, height_) = read_png(path_, viewport_width, viewport_height);

        else if(path_.ends_with(".svg"))
            std::tie(raw_pixel_data, width_, height_) = read_svg(path_, viewport_width, viewport_height, override_r_, override_g_, override_b_);

        else
            sdl_error("Unsupported image type: " + path_);

        texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width_, height_);
        if(!texture_)
            sdl_error("Unable to create SDL texture");

        if(SDL_UpdateTexture(texture_, nullptr, std::data(raw_pixel_data), 4 * width_) < 0)
            sdl_error("Unable to load SDL texture");

        SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(texture_, SDL_ScaleModeBest);
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
        if(!texture_ || !path_.ends_with(".svg"))
            return;

        *this = Texture{renderer, path_, width, height, override_r_, override_g_, override_b_};
    }
}
