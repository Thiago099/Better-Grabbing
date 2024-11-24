#include "InputManager.h"

std::string ToLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool InputManager::ProcessInput(RE::ButtonEvent* button) {
    auto device = button->GetDevice();
    auto idcode = button->GetIDCode();
    auto deviceIterator = inputs.find(device);
    if (deviceIterator == inputs.end()) {
        return false;
    }
    auto deviceData = deviceIterator->second;
    auto inputIterator = deviceData.find(idcode);
    if (inputIterator == deviceData.end()) {
        return false;
    }
    auto actionNames = inputIterator->second;
    bool actionsExecuted = false;
    for (const auto& actionName : actionNames) {
        auto actionIterator = actions.find(actionName);
        if (actionIterator == actions.end()) {
            continue;
        }
        auto& action = actionIterator->second;
        action(button);
        actionsExecuted = true;
    }
    return actionsExecuted;
}
uint32_t InputManager::GetId(std::string key) {
    auto idIterator = idMap.find(key);
    if (idIterator == idMap.end()) {
        auto result = auto_increment;
        idMap[key] = auto_increment++;
        return result;
    }
    return idIterator->second;
}
bool InputManager::HasSink(std::string actionName) {
    actionName = ToLowerCase(actionName);
    auto actionIterator = actions.find(GetId(actionName));
    return actionIterator != actions.end();
}
bool InputManager::HasSource(std::string actionName) {
    actionName = ToLowerCase(actionName);
    auto id = GetId(actionName);
    for (const auto& [device, deviceData] : inputs) {
        for (const auto& [key, keyData] : deviceData) {
            for (const auto& action : keyData) {
                if (action == id) {
                    return true;
                }
            }
        }
    }
    return false;
}

void InputManager::AddSink(std::string actionName, std::function<void(RE::ButtonEvent*)> const& callback) {
    actionName = ToLowerCase(actionName);
    actions.insert(std::make_pair(GetId(actionName), callback));
}

void InputManager::AddSource(std::string actionName, std::string deviceName, std::string buttonName) {
    actionName = ToLowerCase(actionName);
    deviceName = ToLowerCase(deviceName);
    buttonName = ToLowerCase(buttonName);

    auto deviceIterator = deviceMap.find(deviceName);
    if (deviceIterator == deviceMap.end()) {
        return;
    }
    uint32_t device = deviceIterator->second;
    uint32_t idcode;
    switch (device) {
        case RE::INPUT_DEVICE::kKeyboard: {
            auto keyIterator = keyboardMap.find(buttonName);
            if (keyIterator == keyboardMap.end()) {
                return;
            }
            idcode = keyIterator->second;
        } break;
        case RE::INPUT_DEVICE::kMouse: {
            auto keyIterator = mouseMap.find(buttonName);
            if (keyIterator == mouseMap.end()) {
                return;
            }
            idcode = keyIterator->second;
        } break;
        case RE::INPUT_DEVICE::kGamepad: {
            auto keyIterator = gamepadMap.find(buttonName);
            if (keyIterator == gamepadMap.end()) {
                return;
            }
            idcode = keyIterator->second;
        } break;
        default:
            break;
    }
    inputs[device][idcode].push_back(GetId(actionName));
}
