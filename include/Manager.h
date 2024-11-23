#pragma once
#include "Raycast.h"


class Manager {
    bool isGrabbing = false;
    float distance = 200.f;
    inline static Manager* singleton = NULL;
public:
    static Manager* GetSingleton() {
        if (!singleton) {
            singleton = new Manager();
        }
        return singleton;
    }
    void SetGrabbing(bool value) {
        isGrabbing = value;
    }
    void IncrementDistance(float value) {
        auto candidate = distance + value;

        if (candidate < 10.f) {
            candidate = 10.f;
        }

        if (candidate > 1000.f) {
            candidate = 1000.f;
        }

        distance = candidate;
    }
    bool GetIsGrabbing() {
        return isGrabbing;
    }
    void UpdatePosition(RE::TESObjectREFR* obj);
};