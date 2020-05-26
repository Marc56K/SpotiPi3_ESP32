#pragma once
#include "SettingsManager.h"
#include <WebServer.h>
#include <DNSServer.h>

class SetupManager
{
public:
    SetupManager(SettingsManager& settings);
    ~SetupManager();

    const char* GetWifiSsid() const;
    bool SetupCompleted() const;

    void Update();

private:
    void HandleRootRequest();
    void HandleSaveRequest();
    void HandleCssRequest();
    void HandleNotFound();

private:
    DNSServer _dnsServer;
    WebServer _httpServer;
    bool _setupCompleted;
    SettingsManager& _settings;
    std::vector<std::string> _wifiList;

    uint64_t reg_a;
    uint64_t reg_b;
    uint64_t reg_c;
};