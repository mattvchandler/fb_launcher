#ifndef APP_HPP
#define APP_HPP

#include <string>
#include <vector>

struct App
{
    std::string title;
    std::string desc;
    std::string command;
    std::string thumbnail_path;
    std::string bg_path;
    bool input_cec;
    bool input_keyboard;
    bool input_mouse;
    bool input_gamepad;
};

std::vector <App> read_app_list(const std::string & app_list_path);

#endif // APP_HPP
