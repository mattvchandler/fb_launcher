#include <iostream>

#include <cstdlib>

#include "app.hpp"
#include "menu.hpp"

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

    auto selection_index = 0;
    while(true)
    {
        // std::system(apps[selection_index].command.c_str());

        auto menu = Menu{apps};
        // TODO: excpetion handling (what should we even do? most of these ought to be fatal)
        selection_index = menu.run();

        if(menu.get_exited())
            break;
    }
}
