
#pragma once

#include "Settings.hpp"

class ProxyProvider
{
public:
    ProxyProvider(const Settings::Provider& settings) : _settings(settings) {}

    bool isEnabled() const {return !_settings.urls.isEmpty();}

    uint run();

private:
    const Settings::Provider& _settings;
};