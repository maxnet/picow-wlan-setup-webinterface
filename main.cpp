/*
 * Author: Floris Bos
 * License: public domain / unlicense
 */

#include "button.hpp"
#include "pico_explorer.hpp"
#include "pico/bootrom.h"
#include "pico/cyw43_arch.h"
#include "display.h"
#include "setupwebinterface.h"
#include <string>


static void netif_status_callback(struct netif *netif)
{
    if (netif_is_link_up(netif) && netif_ip_addr4(netif))
    {
        std::string msg = std::string("Connected to wifi network.\n"
                                      "IP-address: ")+ip4addr_ntoa(netif_ip_addr4(netif));
        Display::instance()->showText(msg);
    }
}

int main()
{
    SetupWebInterface setupwebinterface;
    pimoroni::Button button_x(PicoExplorer::X);
    pimoroni::Button button_y(PicoExplorer::Y);
    Display *display = Display::instance();

    if (cyw43_arch_init())
    {
        display->showText("Error initializing cyw43");
        return 1;
    }

    if (setupwebinterface.alreadyConfigured())
    {
        Display::instance()->showText("test5");
        std::string msg = std::string("Connecting to: ")+setupwebinterface.savedSSID();
        display->showText(msg);
        setupwebinterface.connectToSavedWlan();
        netif_set_status_callback(netif_default, netif_status_callback);
    }
    else
    {
        /* Scan for wifi networks first as in AP mode chances are it may only stay on 1 channel */
        display->showText("Scanning for wifi networks");
        cyw43_arch_enable_sta_mode();
        cyw43_wifi_pm(&cyw43_state, 0xa11140);        
        WlanScanner::instance()->startScanning();
        for (int i=1; i<3000; i++)
        {
            cyw43_arch_poll();
            sleep_ms(1);            
        }
        display->showText("Starting access point");
        setupwebinterface.startAccessPoint();
    }

    /* Main loop */
    while(true) {

        /* For development purposes, switch to USB programming when Y button is pressed */
        if (button_y.read())
        {
            display->showText("USB programming mode");
            reset_usb_boot(0, 0);
        }
        /* Erase saved WLAN and reboot if X button is pressed */
        if (button_x.read())
        {
            display->showText("Erasing saved settings");
            setupwebinterface.eraseSavedWlan();
        }

        cyw43_arch_poll();
        sleep_ms(1);
    }
}
