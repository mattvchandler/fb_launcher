#include "app.hpp"

#include <stdexcept>

#include <unistd.h>

#include <csvpp/csv.hpp>

std::vector<App> read_app_list(const std::string & app_list_path)
{
    auto apps = std::vector<App>{};

    auto reader = csv::Reader{app_list_path};
    for(auto && row: reader)
    {
        using std::string;
        auto row_t = row.read_tuple<string, string, string, string, int, int, int, int, std::string, int>();

        if(std::get<9>(row_t) == 0) // disabled?
            continue;

        auto thumbnail_path = std::get<3>(row_t);

        if(access(thumbnail_path.c_str(), F_OK) != 0 || access(thumbnail_path.c_str(), R_OK) != 0)
            thumbnail_path.clear();

        apps.emplace_back(App
        {
            .title          = std::get<0>(row_t),
            .desc           = std::get<1>(row_t),
            .command        = std::get<2>(row_t).empty() ? std::string{"/dev/false"} : std::get<2>(row_t),
            .thumbnail_path = std::move(thumbnail_path),
            .input_cec      = std::get<4>(row_t) == 0 ? false : true,
            .input_keyboard = std::get<5>(row_t) == 0 ? false : true,
            .input_mouse    = std::get<6>(row_t) == 0 ? false : true,
            .input_gamepad  = std::get<7>(row_t) == 0 ? false : true,
            .note           = std::get<8>(row_t)
        });
    }

    if(apps.empty())
        throw std::runtime_error{"Error reading app CSV file: no apps listed"};

    return apps;
}
