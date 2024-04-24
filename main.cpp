#include <iostream>
#include <vector>

#include <cstring>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <png.h>
#include <fontconfig/fontconfig.h>

// TODO: drawing context
// TODO: keyboard / joystick input
// TODO: CEC input?
// TODO: load / parse config file
// TODO: unload everything and launch selected program
// TODO: power-off, reboot options
// TODO: autolaunch 1st app (Kodi)
// TODO: RAII cleanup and error handling

struct Fontconfig
{
    FcConfig * config {nullptr};
    Fontconfig()
    {
        if(!FcInit())
            throw std::runtime_error{"Error loading fontconfig library"};
        config = FcInitLoadConfigAndFonts();
    }
    ~Fontconfig()
    {
        FcConfigDestroy(config);
        FcFini();
    }
    operator FcConfig *() { return config; }
    operator const FcConfig *() const { return config; }
};
struct Pattern
{
    FcPattern * pat{nullptr};
    explicit Pattern(FcPattern * pat): pat{pat} {}
    ~Pattern() { if(pat) FcPatternDestroy(pat); }
    operator FcPattern*() { return pat; }
    operator const FcPattern*() const { return pat; }
};
struct FontSet
{
    FcFontSet * set{nullptr};
    explicit FontSet(FcFontSet * set): set{set} {}
    ~FontSet() { if(set) FcFontSetDestroy(set); }
    operator const FcFontSet*() const { return set; }
    operator FcFontSet*() { return set; }
    FcFontSet * operator->() { return set; };
    const FcFontSet * operator->() const { return set; };
    FcPattern* operator[](int i) { return set->fonts[i]; }
    const FcPattern* operator[](int i) const { return set->fonts[i]; }
};



int main(int argc, char * argv[])
{
    if(argc < 2)
    {
        std::cerr<<"Must specify PNG image to display\n";
        return 1;
    }

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        std::cerr<<"Unable to initialize SDL: "<<SDL_GetError()<<'\n';
        SDL_Quit();
        return 1;
    }

    auto window = SDL_CreateWindow("fb_launcher",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if(!window)
    {
        std::cerr<<"Unable to create SDL window: "<<SDL_GetError()<<'\n';
        SDL_Quit();
        return 1;
    }

    auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer)
    {
        std::cerr<<"Unable to create SDL window: "<<SDL_GetError()<<'\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    bool running = true;

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); // TODO: check result

    png_image png;
    std::memset(&png, 0, sizeof(png));
    png.version = PNG_IMAGE_VERSION;

    if(!png_image_begin_read_from_file(&png, argv[1]))
    {
        std::cerr<<"Unable to load png data: "<<png.message<<'\n';
        png_image_free(&png);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    png.format = PNG_FORMAT_RGBA;

    std::vector<unsigned char> raw_pixel_data(PNG_IMAGE_SIZE(png));

    if(!png_image_finish_read(&png, nullptr, std::data(raw_pixel_data), PNG_IMAGE_ROW_STRIDE(png), nullptr))
    {
        std::cerr<<"Unable to load png data: "<<png.message<<'\n';
        png_image_free(&png);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }


    auto tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, png.width, png.height);
    if(!tex)
    {
        std::cerr<<"Unable to create SDL texture: "<<SDL_GetError()<<'\n';
        png_image_free(&png);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    if(SDL_UpdateTexture(tex, nullptr, std::data(raw_pixel_data), 4 * png.width) < 0)
    {
        std::cerr<<"Unable to load SDL texture: "<<SDL_GetError()<<'\n';
        png_image_free(&png);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    png_image_free(&png);
    raw_pixel_data.clear();

    Fontconfig fc;
    Pattern font_pat {FcNameParse(reinterpret_cast<const FcChar8*>("sans-serif"))};
    FcConfigSubstitute(fc, font_pat, FcMatchPattern);
    FcDefaultSubstitute(font_pat);
    FcResult result;
    FontSet fonts {FcFontSort(fc, font_pat, false, NULL, &result)};
    if(result != FcResultMatch)
        throw std::runtime_error{"Error finding font"};
    if(fonts->nfont < 1)
        throw std::runtime_error{"No fonts found"};

    FcChar8 * font_path;
    if(FcPatternGetString(fonts[0], FC_FILE, 0, &font_path) != FcResultMatch)
        throw std::runtime_error{"Could not get font path"};

    if(TTF_Init() < 0)
    {
        std::cerr<<"Unable to init font lib: "<<TTF_GetError()<<'\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    auto font = TTF_OpenFont(reinterpret_cast<const char *>(font_path), 20); // TODO: We'll need some scaling here
    if(!font)
    {
        std::cerr<<"Unable to load font "<<TTF_GetError()<<'\n';
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    auto text = TTF_RenderUTF8_Blended_Wrapped(font, "Testing this\nText", SDL_Color{0xFF, 0xFF, 0xFF, 0xFF}, 0);
    if(!text)
    {
        std::cerr<<"Unable to render font "<<TTF_GetError()<<'\n';
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    auto text_tex = SDL_CreateTextureFromSurface(renderer, text);
    if(!text_tex)
    {
        std::cerr<<"Unable to render font to texture "<<SDL_GetError()<<'\n';
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_FreeSurface(text);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    auto text_dest = SDL_Rect{0, 0, text->w, text->h};


    while(running)
    {
        SDL_Event ev;
        while(SDL_PollEvent(&ev))
        {
            switch(ev.type)
            {
                case SDL_QUIT:
                case SDL_KEYDOWN:
                    running = false;
                    break;
                default:
                    break;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, tex, nullptr, nullptr);
        SDL_RenderCopy(renderer, text_tex, nullptr, &text_dest);
        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    TTF_Quit();

    SDL_FreeSurface(text);
    SDL_DestroyTexture(text_tex);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
