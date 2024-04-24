#include "font.hpp"

#include <fontconfig/fontconfig.h>

namespace {
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
}

namespace SDL
{
    Font::Font(const std::string font_name, int ptsize)
    {
        auto fc = Fontconfig{};
        auto font_pat = Pattern{FcNameParse(reinterpret_cast<const FcChar8*>(font_name.c_str()))};
        FcConfigSubstitute(fc, font_pat, FcMatchPattern);
        FcDefaultSubstitute(font_pat);
        auto result = FcResult{};
        auto found_font = Pattern{FcFontMatch(fc, font_pat, &result)};
        if(result != FcResultMatch)
            throw std::runtime_error{"Error finding font"};

        FcChar8 * font_path;
        if(FcPatternGetString(found_font, FC_FILE, 0, &font_path) != FcResultMatch)
            throw std::runtime_error{"Could not get font path"};

        font = TTF_OpenFont(reinterpret_cast<const char *>(font_path), ptsize);
        if(!font)
            ttf_error("Unable to load font");
    }

    Texture Font::render_text(Renderer & renderer, const std::string & text, SDL_Color color, int wrap_length)
    {
        auto text_surface = Surface{TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), color, wrap_length)};
        if(!text_surface.surface)
            ttf_error("Could not render text");

        return Texture{renderer, text_surface};
    }
}
