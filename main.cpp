#include <iostream>

#include <cstdlib>

#include "menu.hpp"

// TODO: power-off, reboot options

// TODO: probably move to menu class. Maybe isolate SDL to it?
int main(int argc, char * argv[])
{
    // TODO: probably going to need more sophisticated argparse
    if(argc < 2)
    {
        std::cerr<<"Must specify PNG image to display\n";
        return 1;
    }
    if(argc < 3)
    {
        std::cerr<<"Must specify command\n";
        return 1;
    }

    // TODO: load / parse config file

    // TODO: 'select' 1st app at startup (maybe controlled by cmdline args - will need to parse those too in that case)
    // TODO: exit condition!
    while(true)
    {
        // TODO: launch selected app
        // TODO: capture / log output
        std::system(argv[2]);

        auto menu = Menu{argv[1]};
        // TODO: run menu, get selection
        // TODO: excpetion handling (what should we even do? most of these ought to be fatal)
        menu.run();
        break; // TODO: temporary
    }
}

