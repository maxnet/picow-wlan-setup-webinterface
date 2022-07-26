/*
 * Author: Floris Bos
 * License: public domain / unlicense
 */

#include "setupwebinterface.h"
#include "pico/cyw43_arch.h"
#include "hardware/watchdog.h"
#include "hardware/sync.h"
#include "lwip/arch.h"
#include "lwip/netif.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "lwip/apps/httpd.h"
#include "littlefs-lib/pico_hal.h"
#include "display.h"
#include <string>

/* Use the DHCP/DNS server code from TinyUSB */
extern "C" {
#include "pico-sdk/lib/tinyusb/lib/networking/dhserver.h"
#include "pico-sdk/lib/tinyusb/lib/networking/dnserver.h"
}

static std::string _pico_readln(int fd)
{
    std::string s;
    char c;

    while ( pico_read(fd, &c, 1) == 1 )
    {
        if (c == '\n')
            break;
        else if (c != '\r')
            s += c;
    }

    return s;
}

SetupWebInterface::SetupWebInterface()
{
    int rc = pico_mount(false);
    if (rc != LFS_ERR_OK)
    {
        /* Format */
        rc = pico_mount(true);
    }

    int fd = pico_open("wlansettings", LFS_O_RDONLY);
    if (fd >= 0)
    {
        _ssid = _pico_readln(fd);
        _psk = _pico_readln(fd);
    }
    pico_unmount();
}

SetupWebInterface::~SetupWebInterface()
{
}

bool SetupWebInterface::alreadyConfigured()
{
    return !_ssid.empty();
}

std::string SetupWebInterface::savedSSID()
{
    return _ssid;
}

bool SetupWebInterface::connectToSavedWlan()
{
    if (!alreadyConfigured())
        return false;

    cyw43_arch_enable_sta_mode();
    cyw43_wifi_pm(&cyw43_state, 0xa11140);
    cyw43_arch_wifi_connect_async(_ssid.c_str(), _psk.c_str(), CYW43_AUTH_WPA2_MIXED_PSK);

    return true;
}

/* Play captive portal and reply with our own IP-address for any DNS query */
static bool dns_query_proc(const char *name, ip4_addr_t *addr)
{
    static const ip4_addr_t ipaddr = IPADDR4_INIT_BYTES(192, 168, 4, 1);

    *addr = ipaddr;
    return true;
}


u16_t wlanscan_ssi_handler(int iIndex, char *buf, int buflen, u16_t current_tag_part, u16_t *next_tag_part)
{
    static std::string json_buf;

    if (json_buf.empty())
    {
        std::map<std::string, WlanDetails> *ssids = WlanScanner::instance()->getSSIDs();
        json_buf = "[\n";
        bool first = true;

        for (const auto& [ssid, details]: (*ssids) )
        {
            if (first)
                first = false;
            else 
                json_buf += ",";

            json_buf += "{\"ssid\":\""+ssid+"\""
                       +",\"rssi\":"+std::to_string(details.rssi)
                       +",\"needsUsername\":"+(details.needsUsername ? "true" : "false")
                       +",\"needsPassword\":"+(details.needsPassword ? "true" : "false")
                       +"}\n";
        }

        json_buf += "]";
    }

    if (current_tag_part+buflen > json_buf.size() )
        buflen = json_buf.size()-current_tag_part;
    
    memcpy(buf, json_buf.c_str() + current_tag_part, buflen);

    if (current_tag_part+buflen < json_buf.size() )
    {
        *next_tag_part = current_tag_part+buflen;
    }
    else
    {
        json_buf.clear();
    }

    return buflen;
}

static const char *wlanscan_set_wifi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    std::string ssid, password;

    for (int i=0; i<iNumParams; i++)
    {
        if (strcmp(pcParam[i], "ssid") == 0)
        {
            ssid = pcValue[i];
        }
        else if (strcmp(pcParam[i], "password") == 0)
        {
            password = pcValue[i];
        }        
    }

    if (!ssid.empty())
    {
        SetupWebInterface::saveSettings(ssid, password);
    }

    return "/empty.html";
}

void SetupWebInterface::startAccessPoint()
{
    char ssid[33], psk[33];
    std::string qr;
    /* database IP addresses that can be offered to the host; this must be in RAM to store assigned MAC addresses */
    static dhcp_entry_t entries[] =
    {
        /* mac ip address                          lease time */
        { {0}, IPADDR4_INIT_BYTES(192, 168, 4, 2), 24 * 60 * 60 },
        { {0}, IPADDR4_INIT_BYTES(192, 168, 4, 3), 24 * 60 * 60 },
        { {0}, IPADDR4_INIT_BYTES(192, 168, 4, 4), 24 * 60 * 60 },
    };

    static const dhcp_config_t dhcp_config =
    {
        .router = IPADDR4_INIT_BYTES(192, 168, 4, 1),  /* router address (if any) */
        .port = 67,                                /* listen port */
        .dns = IPADDR4_INIT_BYTES(192, 168, 4, 1), /* dns server (if any) */
        "",                                        /* dns suffix */
        3,                                         /* num entry */
        entries                                    /* entries */
    };

    static const char * ssi_tags[] = {
        "wlanscan"
    };
    static const tCGI cgi_handlers[] = {
        {"/wifi/setnetwork", wlanscan_set_wifi_handler}
    };

    snprintf(ssid, sizeof(ssid), "picow-%02x%02x%02x%02x%02x%02x",
        netif_default->hwaddr[0], netif_default->hwaddr[1], netif_default->hwaddr[2],
        netif_default->hwaddr[3], netif_default->hwaddr[4], netif_default->hwaddr[5]);
    snprintf(psk, sizeof(psk), "%08x%08x%08x%08x", LWIP_RAND(), LWIP_RAND(), LWIP_RAND(), LWIP_RAND() );

    cyw43_arch_enable_ap_mode(ssid, psk, CYW43_AUTH_WPA2_AES_PSK);
    cyw43_wifi_pm(&cyw43_state, 0xa11140);
    while (dhserv_init(&dhcp_config) != ERR_OK);
    while (dnserv_init(IP_ADDR_ANY, 53, dns_query_proc) != ERR_OK);
    httpd_init();
    http_set_ssi_handler(wlanscan_ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
    http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));

    qr = std::string("WIFI:S:")+ssid+";T:WPA;P:"+psk+";;";
    Display::instance()->showQr(qr);
}

static void _reboot()
{
    Display::instance()->showText("Rebooting...");
    /* Reboot after half a second, so that webserver has time to finish sending response */
    watchdog_reboot(0, SRAM_END, 500);
}

void SetupWebInterface::saveSettings(const std::string &ssid, const std::string &psk)
{
    Display::instance()->showText("Saving settings...");

    pico_mount(false);
    int fd = pico_open("wlansettings", LFS_O_WRONLY | LFS_O_CREAT);
    std::string buf = ssid+"\n"+psk+"\n";
    pico_write(fd, buf.c_str(), buf.size());
    pico_close(fd);
    pico_unmount();

    _reboot();
}

void SetupWebInterface::eraseSavedWlan()
{
    pico_mount(true);
    pico_unmount();
    _reboot();
}

