#pragma once
#include "Raycast.h"


class Manager {
    bool isGrabbing = false;
    float distance = 140.f;
    RE::NiPoint2 angle = {0, 0};
    RE::NiPoint2 position = {0, 0};
    inline static Manager* singleton = NULL;
    void UpdateObjectTransform(RE::TESObjectREFR* obj, RE::NiPoint3& rayPosition);
public:
    static Manager* GetSingleton() {
        if (!singleton) {
            singleton = new Manager();
        }
        return singleton;
    }
    void SetGrabbing(bool value, RE::TESObjectREFRPtr ref) {
        if (value) {
            angle = {0, 0};
            distance = 140.f;
            position = {0, 0};
            if (ref) {
                if (auto ref2 = ref.get()) {
                    auto [cameraAngle, cameraPosition] = RayCast::GetCameraData();
                    auto objectAngle = ref2->GetAngle();
                    angle = {-objectAngle.z + cameraAngle.z, 0};
                }
            }
        }
        isGrabbing = value;
    }
    void RotateX(float x) {
        angle.x += x;
    }
    void RotateY(float y) {
        angle.y += y;
    }
    void TranslateX(float x) {
        position.x += x;
    }
    void TranslateY(float y) {
        position.y += y;
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