#include <iostream>
#include <stdexcept>

#include <cstdlib>

#include "app.hpp"
#include "menu.hpp"

void usage()
{
    std::cout<<"Usage: fb_launcher [-l] [-e] [-c COMMAND] [-h] APP_LIST_CSV\n"
               "Display a launcher for a set of apps (defined in APP_LIST_CSV)\n"
               "Can be run from the linux console without X or Wayland,\n"
               "and can be controlled with keyboard, gamepad, or at TV remote via CEC\n"
               "\n"
               "Arguments\n"
               "  -l             Launch first program in list without displaying launcher\n"
               "  -e             Enable pressing escape to quit\n"
               "  -c             Set a command to be executed on pressing Ctrl+Shift+Esc\n"
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
               "  Note:           Text displayed next to input icons. Can be used to specify\n"
               "                  number of players. May be left blank\n"
               "  Enabled:        1 to display this app in the menu, 0 to hide it\n"
               "\n"
               "The file should not contain a header row\n"
               "\n"
               "Example file contents:\n"
               "Firefox,Browse the World Wide Web,firefox,/usr/share/icons/hicolor/128x128/apps/firefox.png,0,1,1,0,,1\n"
               "Chess,Play the classic two-player board game of chess,/usr/games/gnome-chess,/usr/share/icons/hicolor/scalable/apps/org.gnome.Chess.svg,0,1,1,0,1-2 players,1\n";
}

int main(int argc, char * argv[])
{
    auto selection_index = -1;
    auto allow_escape = false;
    auto ctrl_alt_del_cmd = std::string{};

    for(int i = 1; i < argc;)
    {
        if(argv[i][0] == '-')
        {
            auto nargs = 1;
            switch(argv[i][1])
            {
                case 'l':
                    selection_index = 0;
                    break;

                case 'c':
                    if(i + 1 >= argc)
                    {
                        usage();
                        std::cerr<<"\n-c requires argument\n";
                        return 1;
                    }

                    nargs = 2;
                    ctrl_alt_del_cmd = argv[i + 1];
                    break;

                case 'e':
                    allow_escape = true;
                    break;

                case 'h':
                    usage();
                    return 0;

                default:
                    usage();
                    std::cerr<<"\nUnknown argument '" << argv[i][1] <<"'\n";
                    return 1;
            }

            for(int j = i; j < argc; ++j)
                argv[j] = argv[j + nargs];
            argc -= nargs;
        }
        else
            ++i;
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
            {
                std::cout<<"Launching "<<apps[selection_index].title<<" ("<<apps[selection_index].command<<")\n";
                std::cout.flush();
                std::system(apps[selection_index].command.c_str());
            }

            std::cout<<"Loading menu...\n";
            auto menu = Menu{apps, allow_escape, selection_index, ctrl_alt_del_cmd};

            selection_index = menu.run();
            std::cout<<"Exiting menu...\n";

            if(menu.get_exited())
                break;
        }
    }
    catch(const std::runtime_error & e)
    {
        std::cerr<<e.what()<<'\n';
        return 1;
    }
}
