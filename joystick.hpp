#ifndef JOYSTICK_HPP
#define JOYSTICK_HPP

#include <variant>
#include <vector>

#include <SDL2/SDL.h>

namespace SDL
{
    // TODO: probably will need a way to expose which type it is and get the appropriate internal object
    // We can always get an SDL_Joystick from an SDL_GameController, so maybe the interface should reflect that
    class Joystick
    {
    private:
        std::variant<SDL_Joystick*, SDL_GameController*> joystick_ {static_cast<SDL_Joystick*>(nullptr)};
        std::vector<char> centered_;
    public:

        enum class Dir {NONE, PREV, NEXT};

        explicit Joystick(int index);

        Joystick(const Joystick &) = delete;
        Joystick &operator=(const Joystick &) = delete;
        Joystick(Joystick && j);
        Joystick &operator=(Joystick && j);
        ~Joystick();

        Dir menu_move(const SDL_Event & ev);
        const char * name();

        SDL_GameController * gc();
        const SDL_GameController * gc() const;

        SDL_Joystick * joy();
        const SDL_Joystick * joy() const;

        bool is_gc() const { return std::holds_alternative<SDL_GameController*>(joystick_); }
    };
}
#endif // JOYSTICK_HPP
