#include "Translations.h"

#include <nlohmann/json.hpp>

namespace Translations {
    constexpr const char* translationsFile = "Data/SKSE/Plugins/BetterGrabbingStrings.json";
    constexpr const char* defaultTranslation = "missing translation";
    std::map<std::string, std::string> translations;
}

void Translations::Install() {
    std::ifstream file(translationsFile);
    if (!file.is_open()) {
        logger::error("Could not open translation file: {}", translationsFile);
        return;
    }

    const auto document = nlohmann::json::parse(file, nullptr, false);
    if (!document.is_object()) {
        logger::error("Translation file must contain a valid JSON object: {}", translationsFile);
        return;
    }

    translations.clear();
    for (const auto& [key, value] : document.items()) {
        if (!value.is_string()) {
            logger::warn("Ignoring non-string translation: {}", key);
            continue;
        }
        translations[key] = value.get<std::string>();
    }

    logger::info("Loaded {} translations from {}", translations.size(), translationsFile);
}

const char* Translations::Get(const std::string& key) {
    IF_FIND(translations, key, it) {
        return it->second.c_str();
    }

    logger::warn("Missing translation: {}", key);
    return defaultTranslation;
}
