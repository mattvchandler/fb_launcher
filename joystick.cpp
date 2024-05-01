#include "joystick.hpp"

#include <algorithm>
#include <cmath>
#include <variant>

#include "sdl.hpp"

namespace SDL
{
    Joystick::Joystick(int index) : joystick_{SDL_JoystickOpen(index)}
    {
        if(SDL_IsGameController(index))
        {
            auto g = SDL_GameControllerOpen(index);
            if(!g)
                sdl_error("Unable to create SDL gamecontroller");
            joystick_ = g;

            centered_.resize(SDL_CONTROLLER_AXIS_MAX);
            std::fill(std::begin(centered_), std::end(centered_), 1);
        }
        else
        {
            auto j = SDL_JoystickOpen(index);
            if(!j)
                sdl_error("Unable to create SDL joystick");

            centered_.resize(1);
            std::fill(std::begin(centered_), std::end(centered_), 1);
            joystick_ = j;
        }
    }
    Joystick::Joystick(Joystick && j):
        joystick_{j.joystick_},
        centered_{std::move(j.centered_)}
    {
        j.joystick_ = static_cast<SDL_Joystick*>(nullptr);
    }
    Joystick & Joystick::operator=(Joystick && j)
    {
        if(this != &j)
        {
            joystick_ = j.joystick_;
            j.joystick_ = static_cast<SDL_Joystick*>(nullptr);
            centered_ = std::move(j.centered_);
        }
        return *this;
    }
    Joystick::~Joystick()
    {
        if(std::holds_alternative<SDL_Joystick*>(joystick_))
        {
            auto & j = std::get<SDL_Joystick*>(joystick_);
            if(j)
                SDL_JoystickClose(j);
        }
        else
        {
            auto & g = std::get<SDL_GameController*>(joystick_);
            if(g)
                SDL_GameControllerClose(g);
        }
    }

    Joystick::Dir Joystick::menu_move(const SDL_Event & ev)
    {
        char * ctr = nullptr;
        Sint16 x = 0, y = 0;

        if(ev.type == SDL_JOYAXISMOTION && std::holds_alternative<SDL_Joystick* >(joystick_))
        {
            if(ev.jaxis.axis >= 2)
                return Dir::NONE;

            auto & j = std::get<SDL_Joystick*>(joystick_);

            x = SDL_JoystickGetAxis(j, 0);
            y = SDL_JoystickGetAxis(j, 1);
            ctr = &centered_[0];
        }
        else if(ev.type == SDL_CONTROLLERAXISMOTION && std::holds_alternative<SDL_GameController*>(joystick_))
        {
            // We're tracking centering for both axes in the centered_[...X] var
            auto & c = std::get<SDL_GameController*>(joystick_);
            switch(ev.caxis.axis)
            {
                case SDL_CONTROLLER_AXIS_LEFTX:
                case SDL_CONTROLLER_AXIS_LEFTY:
                    ctr = &centered_[SDL_CONTROLLER_AXIS_LEFTX];
                    x = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTX);
                    y = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTY);
                    break;
                case SDL_CONTROLLER_AXIS_RIGHTX:
                case SDL_CONTROLLER_AXIS_RIGHTY:
                    ctr = &centered_[SDL_CONTROLLER_AXIS_RIGHTX];
                    x = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_RIGHTX);
                    y = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_RIGHTY);
                    break;
                case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                    ctr = &centered_[ev.caxis.axis];
                    x = ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT ? ev.caxis.value : -ev.caxis.value;
                    break;
            }
        }
        else
            return Dir::NONE;

        auto len2 = x*x + y*y;
        constexpr auto deadzone = 8'000;
        constexpr auto threshold = 20'000;
        constexpr auto deadzone2 = deadzone*deadzone;
        constexpr auto threshold2 = threshold*threshold;

        if(len2 > threshold2 && *ctr)
        {
            *ctr = false;
            if(std::abs(x) > std::abs(y))
                return x < 0 ? Dir::PREV : Dir::NEXT;
            else
                return y < 0 ? Dir::PREV : Dir::NEXT;
        }
        else if(len2 < deadzone2)
        {
            *ctr = true;
            return Dir::NONE;
        }
        else
            return Dir::NONE;
    }

    const char * Joystick::name()
    {
        if(std::holds_alternative<SDL_Joystick*>(joystick_))
            return SDL_JoystickName(std::get<SDL_Joystick*>(joystick_));
        else
            return SDL_GameControllerName(std::get<SDL_GameController*>(joystick_));
    }

    SDL_GameController * Joystick::gc()
    {
        if(auto gc = std::get_if<SDL_GameController*>(&joystick_); gc)
            return *gc;
        else
            return nullptr;
    }
    const SDL_GameController * Joystick::gc() const
    {
        if(auto gc = std::get_if<SDL_GameController*>(&joystick_); gc)
            return *gc;
        else
            return nullptr;
    }

    SDL_Joystick * Joystick::joy()
    {
        if(auto gc = std::get_if<SDL_GameController*>(&joystick_); gc)
            return SDL_GameControllerGetJoystick(*gc);
        else
            return std::get<SDL_Joystick*>(joystick_);
    }
    const SDL_Joystick * Joystick::joy() const
    {
        if(auto gc = std::get_if<SDL_GameController*>(&joystick_); gc)
            return SDL_GameControllerGetJoystick(*gc);
        else
            return std::get<SDL_Joystick*>(joystick_);
    }
}
