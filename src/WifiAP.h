#include <WiFi.h>

class WifiAP
{
public:
    WifiAP();
    ~WifiAP();

    void Start(const char* ssid, const char* password);
    void Update();

private:
    WiFiServer _server;
};