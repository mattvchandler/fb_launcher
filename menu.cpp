#include "menu.hpp"
#include "sdl.hpp"
#include "texture.hpp"

#include <iostream> // TODO: delete

namespace
{
    // units are percentages
    constexpr auto row_height = 20;
    constexpr auto horiz_margin = 10;
    constexpr auto row_spacing = 10;
    constexpr auto col_spacing = 5;

    constexpr auto text_color = SDL_Color {0xFF, 0xFF, 0xFF, 0xFF};
}

void Menu::prev()
{
    std::cout<<"Prev menu item stub\n";
}

void Menu::next()
{
    std::cout<<"Next menu item stub\n";
}

void Menu::select()
{
    std::cout<<"Menu select stub\n";
}

void Menu::draw(SDL::Renderer & renderer, SDL::Texture & tex)
{
    if(w_ == 0 || h_ == 0)
        return;

    // TODO: BG image (blended with black?)
    draw_row(0, renderer, tex);
    draw_row(-1, renderer, tex);
    draw_row(1, renderer, tex);
}

void Menu::resize(int w, int h)
{
    w_ = w; h_ = h;

    auto font_size = h / 20;

    std::cout<<"Font sizes: "<<font_size<<" "<<font_size/2<<'\n';

    title_font_ = SDL::Font{"sans-serif", font_size};
    desc_font_ = SDL::Font{"sans-serif", font_size / 2};
}

void Menu::draw_row(int pos, SDL::Renderer & renderer, SDL::Texture & tex)
{
    const auto row_height_px = row_height * h_ / 100;
    const auto horiz_margin_px = horiz_margin * w_ / 100;
    const auto row_spacing_px = row_spacing * h_ / 100;
    const auto col_spacing_px = col_spacing * w_ / 100;

    const auto image_size_px = row_height_px;
    const auto row_top_px = h_ / 2 + pos * (row_height_px + row_spacing_px) - row_height_px / 2;
    const auto text_x = horiz_margin_px + image_size_px + col_spacing_px;

    // TODO: get text from file, and cache texture
    auto title_text = title_font_.render_text(renderer, "Menu item # 1", text_color);
    auto desc_text = desc_font_.render_text(renderer, "This is the description for my menu item. It is rather long and ought to wrap so I'm going to keep typing to make sure that it does and this is probably enough text that it will wrap so I'm going to stop now.",
            text_color, w_ - (2 * horiz_margin_px + image_size_px + col_spacing_px));

    const auto fade = pos == 0 ? 255 : 64;
    SDL_SetTextureColorMod(tex, fade, fade, fade);
    SDL_SetTextureColorMod(title_text, fade, fade, fade);
    SDL_SetTextureColorMod(desc_text, fade, fade, fade);

    tex.render(renderer, horiz_margin_px, row_top_px, image_size_px, image_size_px);
    title_text.render(renderer, text_x, row_top_px);
    desc_text.render(renderer, text_x, row_top_px + title_text.get_height());
}
