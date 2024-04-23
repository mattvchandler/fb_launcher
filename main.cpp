#include <iostream>

#include <SDL.h>

// TODO: drawing context
// TODO: keyboard / joystick input
// TODO: CEC input?
// TODO: load / parse config file
// TODO: unload everything and launch selected program
// TODO: power-off, reboot options
// TODO: autolaunch 1st app (Kodi)

int main(int argc, char * argv[])
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        std::cerr<<"Unable to initialize SDL: "<<SDL_GetError()<<'\n';
        SDL_Quit();
        return 1;
    }

    auto window = SDL_CreateWindow("fb_launcher",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600,
            SDL_WINDOW_OPENGL);
    if(!window)
    {
        std::cerr<<"Unable to create SDL window: "<<SDL_GetError()<<'\n';
        SDL_Quit();
        return 1;
    }

    auto surface = SDL_GetWindowSurface(window);
    if(!surface)
    {
        std::cerr<<"Unable to get SDL window surface: "<<SDL_GetError()<<'\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if(SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0x00, 0xFF, 0x00)) < 0)
    {
        std::cerr<<"Unable to fill surface: "<<SDL_GetError()<<'\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    bool running = true;
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

        // SDL_GL_SwapWindow(window);
        if(SDL_UpdateWindowSurface(window) < 0)
        {
            std::cerr<<"Unable to update window surface: "<<SDL_GetError()<<'\n';
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
