#pragma once
#include <WebServer.h>
#include <DNSServer.h>

class SetupManager
{
public:
    SetupManager();
    ~SetupManager();

    const char* GetWifiSsid() const;
    const char* GetWifiKey() const;
    bool SetupCompleted() const;

    void Update();

private:
    void HandleRootRequest();
    void HandleSaveRequest();
    void HandleCssRequest();
    void HandleNotFound();

private:
    String _wifiKey;
    DNSServer _dnsServer;
    WebServer _httpServer;
    bool _setupCompleted;

    uint64_t reg_a;
    uint64_t reg_b;
    uint64_t reg_c;
};