#include <iostream>
#include <vector>

#include <cerrno>
#include <cstring>

#include <SDL.h>
#include <png.h>

// TODO: drawing context
// TODO: keyboard / joystick input
// TODO: CEC input?
// TODO: load / parse config file
// TODO: unload everything and launch selected program
// TODO: power-off, reboot options
// TODO: autolaunch 1st app (Kodi)

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
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, 0);
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
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
