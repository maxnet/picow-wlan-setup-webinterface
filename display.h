/*
 * Author: Floris Bos
 * License: public domain / unlicense
 */

#pragma once

#include "pico_explorer.hpp"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "font8_data.hpp"
#include <string>

class Display {
    public:
        static Display *instance();
        void showText(const char *msg);
        void showText(const std::string &msg);
        void showQr(const char *msg);
        void showQr(const std::string &msg);

    protected:
        static Display *_display;
        pimoroni::ST7789 st7789;
        pimoroni::Pen BLACK;
        pimoroni::Pen WHITE;
        pimoroni::PicoGraphics_PenP4 graphics;

        Display();
};
