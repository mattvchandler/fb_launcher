#ifndef MENU_HPP
#define MENU_HPP

#include "font.hpp"
#include "sdl.hpp"
#include "texture.hpp"

class Menu
{
public:
    void prev();
    void next();
    void select();
    void draw(SDL::Renderer & renderer, SDL::Texture & tex); // TODO: tex is temporary
    void resize(int w, int h);

private:
    int w_{0}, h_{0};

    SDL::Font title_font_;
    SDL::Font desc_font_;

    void draw_row(int pos, SDL::Renderer & renderer, SDL::Texture & tex);

};

#endif // MENU_HPP
