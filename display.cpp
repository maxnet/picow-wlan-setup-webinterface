/*
 * Author: Floris Bos
 * License: public domain / unlicense
 */

#include "display.h"

/* Using the C implementation instead of C++ because we have exceptions disabled */
#include "QR-Code-generator/c/qrcodegen.h"

Display *Display::_display = NULL;

using namespace pimoroni;

Display *Display::instance()
{
    if (!_display)
        _display = new Display();

    return _display;
}

Display::Display()
 : st7789(PicoExplorer::WIDTH, PicoExplorer::HEIGHT, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT)),
          graphics(st7789.width, st7789.height, nullptr)
{
    BLACK = graphics.create_pen(0, 0, 0);
    WHITE = graphics.create_pen(255, 255, 255);
    st7789.set_backlight(100);
    graphics.set_pen(BLACK);
    graphics.clear();
    st7789.update(&graphics);
}

void Display::showText(const char *msg)
{
    graphics.set_pen(BLACK);
    graphics.clear();
    graphics.set_pen(WHITE);
    graphics.set_font(&font8);
    graphics.text(msg, Point(10, 10), 220);
    st7789.update(&graphics);
}

void Display::showText(const std::string &msg)
{
    showText(msg.c_str());
}

void Display::showQr(const char *msg)
{
    int buflen = qrcodegen_BUFFER_LEN_FOR_VERSION(10);
    uint8_t qrcode[buflen];
    uint8_t tempBuffer[buflen];

    bool ok = qrcodegen_encodeText(msg, tempBuffer, qrcode, qrcodegen_Ecc_MEDIUM,
        qrcodegen_VERSION_MIN, 10, qrcodegen_Mask_AUTO, true);
    if (ok)
    {
        int size = qrcodegen_getSize(qrcode);
        int minborder = 10;
        int blocksize = (PicoExplorer::HEIGHT-minborder*2)/size;
        int border = (PicoExplorer::HEIGHT-(size*blocksize))/2;

        graphics.set_pen(WHITE);
        graphics.clear();
        graphics.set_pen(BLACK);

        for (int y=0; y < size; y++)
        {
            for (int x=0; x < size; x++)
            {
                if (qrcodegen_getModule(qrcode, x, y))
                {
                    graphics.rectangle(Rect(x*blocksize+border, y*blocksize+border, blocksize, blocksize));
                }
            }
        }
        st7789.update(&graphics);
    }
    else
    {
        showText("Error generating QR code");
    }
}

void Display::showQr(const std::string &msg)
{
    showQr(msg.c_str());
}
