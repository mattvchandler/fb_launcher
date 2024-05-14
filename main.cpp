#include <iostream>
#include <stdexcept>

#include <cstdlib>

#include "app.hpp"
#include "menu.hpp"

void usage()
{
    std::cout<<"Usage: fb_launcher [-l] [-h] APP_LIST_CSV\n"
               "Display a launcher for a set of apps (defined in APP_LIST_CSV)\n"
               "Can be run from the linux console without X or Wayland,\n"
               "and can be controlled with keyboard, gamepad, or at TV remote via CEC\n"
               "\n"
               "Arguments\n"
               "  -l             Launch first program in list without displaying launcher\n"
               "  -h             Display this message and exit\n"
               "  APP_LIST_CSV   A CSV file containing the list of apps to display\n"
               "                 See below for file format\n"
               "\n"
               "CSV file columns\n"
               "  Title:          Name of program\n"
               "  Description:    Description of program to display under the name\n"
               "  Command:        Command to be executed when selected from the menu\n"
               "  Thumbnail:      Path to an image (PNG or SVG) to display for the program.\n"
               "                  Usually the application icon\n"
               "  CEC input:      1 if input via CEC is supported by this program, else 0.\n"
               "                  An icon will be displayed for each input type listed with a 1\n"
               "  Keyboard input: 1 if keyboard input is supported by this program, else 0\n"
               "  Mouse input:    1 if mouse input is supported by this program, else 0\n"
               "  Gamepad input:  1 if gamepad input is supported by this program, else 0\n"
               "\n"
               "The file should not contain a header row\n"
               "\n"
               "Example file contents:\n"
               "Firefox,Browse the World Wide Web,firefox,/usr/share/icons/hicolor/128x128/apps/firefox.png,0,1,1,0\n";
}

int main(int argc, char * argv[])
{
    auto selection_index = -1;

    for(int i = 0; i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            switch(argv[i][1])
            {
                case 'l':
                    selection_index = 0;
                    break;

                case 'h':
                    usage();
                    return 0;

                default:
                    usage();
                    std::cout<<"Unknown argument '" << argv[i][1] <<"'\n";
                    return 1;
            }

            argc -= 1;
            for(int j = i; j < argc; ++j)
                argv[j] = argv[j + 1];
        }
    }

    if(argc < 2)
    {
        usage();
        std::cerr<<"Must specify app CSV file\n";
        return 1;
    }
    if(argc > 2)
    {
        usage();
        std::cerr<<"Too many arguments\n";
        return 1;
    }

    try
    {
        auto apps = read_app_list(argv[1]);

        while(true)
        {
            if(selection_index >= 0 && selection_index < static_cast<int>(std::size(apps)))
                std::system(apps[selection_index].command.c_str());

            auto menu = Menu{apps};

            selection_index = menu.run();

            if(menu.get_exited())
                break;
        }
    }
    catch(const std::runtime_error & e)
    {
        std::cerr<<e.what();
        return 1;
    }
}
