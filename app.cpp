#include "app.hpp"

#include <csvpp/csv.hpp>
#include <stdexcept>

std::vector<App> read_app_list(const std::string & app_list_path)
{
    auto apps = std::vector<App>{};

    auto reader = csv::Reader{app_list_path};
    for(auto && row: reader)
    {
        using std::string;
        auto row_t = row.read_tuple<string, string, string, string, int, int, int, int, std::string>();

        apps.emplace_back(App
        {
            .title          = std::get<0>(row_t).empty() ? std::string{" "} : std::get<0>(row_t),
            .desc           = std::get<1>(row_t).empty() ? std::string{" "} : std::get<1>(row_t),
            .command        = std::get<2>(row_t).empty() ? std::string{"/dev/false"} : std::get<2>(row_t),
            .thumbnail_path = std::get<3>(row_t),
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
