#ifndef CONFIG_SINGLETON_H
#define CONFIG_SINGLETON_H

#include "Config.h"

class ConfigSingleton {
public:
    static Config& getInstance() {
        static Config instance;
        static bool loaded = false;
        if (!loaded) {
            instance.load("/etc/gnudialer.conf");
            loaded = true;
        }
        return instance;
    }

private:
    ConfigSingleton() {}
    ConfigSingleton(const ConfigSingleton&) = delete;
    ConfigSingleton& operator=(const ConfigSingleton&) = delete;
};

#endif // CONFIG_SINGLETON_H
