# FB launcher

An app launcher that can run on the linux framebuffer, with no need for X or
wayland.

Especially useful for Media PCs

## Dependencies

* SDL2
* SDL2 TFF
* libpng
* librsvg
* fontconfig
* libcec
* cmake
* pkg-config
* csvpp (included as a submodule)

On Debian, these can all be installed with:

    sudo apt install build-essential git cmake pkg-config libsdl2-dev libsdl2-ttf-dev libpng-dev librsvg2-dev libfontconfig-dev libcec-dev

## Building

    git clone --recurse-submodules https://github.com/mattvchandler/fb_launcher
    cd fb_launcher
    cmake -S . -B build
    cmake --build build -j4 # or however many CPU cores you care to allocate
    build/fb_launcher <YOUR CSV FILE HERE - See below>

## CSV file format

#### CSV file columns
* Title:          Name of program
* Description:    Description of program to display under the name
* Command:        Command to be executed when selected from the menu
* Thumbnail:      Path to an image (PNG or SVG) to display for the program.
                  Usually the application icon
* CEC input:      1 if input via CEC is supported by this program, else 0.
                  An icon will be displayed for each input type listed with a 1
* Keyboard input: 1 if keyboard input is supported by this program, else 0
* Mouse input:    1 if mouse input is supported by this program, else 0
* Gamepad input:  1 if gamepad input is supported by this program, else 0
* Note:           Text displayed next to input icons. Can be used to specify
                  number of players. May be left blank
* Enabled:        1 to display this app in the menu, 0 to hide it

The file should not contain a header row

#### Example file contents:
    Firefox,Browse the World Wide Web,/usr/bin/firefox,/usr/share/icons/hicolor/128x128/apps/firefox.png,0,1,1,0,,1
    Chess,Play the classic two-player board game of chess,/usr/games/gnome-chess,/usr/share/icons/hicolor/scalable/apps/org.gnome.Chess.svg,0,1,1,0,1-2 players,1

Keyboard, Mouse, Game Controller, and Remote Control icons by [Font Awesome](https://fontawesome.com/license/free) (CC BY 4.0) Copyright 2024 Fonticons, Inc.
