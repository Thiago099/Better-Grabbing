#include "Ini.h"

void Ini::IniReader::GetKeyPair(const char* section, std::function<void(const char*, const char*)> const& fn) const {
    if (!opened) return;
    CSimpleIniA::TNamesDepend keys;
    _ini.GetAllKeys(section, keys);
    for (auto it = keys.begin(); it != keys.end(); ++it) {
        const char* key = it->pItem;
        const char* value = _ini.GetValue(section, key);
        fn(key, value);
    }
}
