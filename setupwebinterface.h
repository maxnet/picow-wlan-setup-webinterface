/*
 * Author: Floris Bos
 * License: public domain / unlicense
 */

#pragma once

#include "hardware/flash.h"
#include "wlanscanner.h"
#include <string>

class SetupWebInterface
{
    public:
        SetupWebInterface();
        virtual ~SetupWebInterface();
        bool alreadyConfigured();
        std::string savedSSID();        
        bool connectToSavedWlan();
        void startAccessPoint();
        void eraseSavedWlan();
        static void saveSettings(const std::string &ssid, const std::string &psk);

    protected:
        std::string _ssid, _psk;
};
