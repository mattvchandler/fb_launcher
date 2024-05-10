#include <iostream>

#include <cstdlib>

#include "app.hpp"
#include "menu.hpp"

// TODO: power-off, reboot options

// TODO: probably move to menu class. Maybe isolate SDL to it?
int main(int argc, char * argv[])
{
    // TODO: probably going to need more sophisticated argparse
    if(argc < 2)
    {
        std::cerr<<"Must specify app CSV file\n";
        return 1;
    }

    // load / parse config file
    auto apps = read_app_list(argv[1]);

    // TODO: 'select' 1st app at startup (maybe controlled by cmdline args - will need to parse those too in that case)
    // TODO: exit condition!

    while(true)
    {
        // TODO: launch selected app
        // TODO: capture / log output
        // std::system(apps[selection_index].command);

        auto menu = Menu{apps};
        // TODO: run menu, get selection
        // TODO: excpetion handling (what should we even do? most of these ought to be fatal)
        menu.run();
        break; // TODO: temporary
    }
}
