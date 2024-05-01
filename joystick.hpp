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
        std::variant<SDL_Joystick*, SDL_GameController*> joystick {static_cast<SDL_Joystick*>(nullptr)};
        std::vector<char> centered;
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

    };
}
#endif // JOYSTICK_HPP
