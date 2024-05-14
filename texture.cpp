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

    std::tuple<std::vector<unsigned char>, int, int> read_png(const std::string & png_path)
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

        return {raw_pixel_data, png.width, png.height};
    }

    std::tuple<std::vector<unsigned char>, int, int> read_svg(const std::string & svg_path,
            int suggested_width, int suggested_height,
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

        int width{0}, height{0};
    #if LIBRSVG_MAJOR_VERSION > 2 || (LIBRSVG_MAJOR_VERSION == 2 && LIBRSVG_MINOR_VERSION >= 52)
        gdouble intrinsic_width{0.0}, intrinsic_height{0.0};
        rsvg_handle_get_intrinsic_size_in_pixels(handle, &intrinsic_width, &intrinsic_height);
        width = intrinsic_width;
        height = intrinsic_height;
        if(width == 0 || height == 0)
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

        if(suggested_width > 0 && suggested_height > 0)
        {
            auto ratio = static_cast<float>(width) / height;
            if(ratio > 1.0f)
            {
                width = ratio * suggested_height;
                height = suggested_height;
            }
            else
            {
                width = suggested_width;
                height = ratio * suggested_width;
            }
        }

        cairo_surface_t * surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
        rs.push(surface, cairo_surface_destroy);
        if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
            throw std::runtime_error { "Error creating SVG cairo surface" };

        cairo_t * cr = cairo_create(surface);
        rs.push(cr, cairo_destroy);
        if(cairo_status(cr) != CAIRO_STATUS_SUCCESS)
            throw std::runtime_error {"Error creating SVG cairo object"};

    #if LIBRSVG_MAJOR_VERSION > 2 || (LIBRSVG_MAJOR_VERSION == 2 && LIBRSVG_MINOR_VERSION >= 52)
        auto viewport = RsvgRectangle {.x=0.0, .y=0.0, .width=static_cast<double>(width), .height=static_cast<double>(height)};
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

        std::vector<unsigned char> raw_pixel_data(width * height * 4);

        if(cairo_image_surface_get_stride(surface) < width * 4)
            throw std::runtime_error {"Invalid SVG stride"};

        auto * surface_data = cairo_image_surface_get_data(surface);
        for(int row = 0; row < height; ++row)
            std::memcpy(std::data(raw_pixel_data) + row * width * 4, surface_data + cairo_image_surface_get_stride(surface) * row, width * 4);

        return {raw_pixel_data, width, height};
    }
}

namespace SDL
{
    Texture::Texture(Renderer & renderer, const std::string & img_path,
            int suggested_width, int suggested_height,
            int override_r, int override_g, int override_b):
        path_(img_path),
        override_r_{override_r},
        override_g_{override_g},
        override_b_{override_b}
    {
        auto raw_pixel_data = std::vector<unsigned char>{};

        if(img_path.ends_with(".png"))
        {
            std::tie(raw_pixel_data, width_, height_) = read_png(img_path);
        }
        else if(img_path.ends_with(".svg"))
        {
            std::tie(raw_pixel_data, width_, height_) = read_svg(img_path, suggested_width, suggested_height, override_r_, override_g_, override_b_);
            rescalable_ = true;
        }
        else
            sdl_error("Unsupported image type: " + img_path);

        texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width_, height_);
        if(!texture_)
            sdl_error("Unable to create SDL texture");

        if(SDL_UpdateTexture(texture_, nullptr, std::data(raw_pixel_data), 4 * width_) < 0)
            sdl_error("Unable to load SDL texture");
    }


    void Texture::render(Renderer & renderer, int x, int y, int size_w, int size_h)
    {
        if(!texture_)
            return;

        render_dest_.x = x;
        render_dest_.y = y;
        render_dest_.w = size_w ? size_w : width_;
        render_dest_.h = size_h ? size_h : height_;
        SDL_RenderCopy(renderer, texture_, nullptr, &render_dest_);
    }

    void Texture::rescale(Renderer & renderer, int width, int height)
    {
        if(!rescalable_)
            return;

        *this = Texture{renderer, path_, width, height, override_r_, override_g_, override_b_};
    }
}
