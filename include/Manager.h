#pragma once
#include "Raycast.h"
#include "InputManager.h"

class Manager {
    bool isGrabbing = false;
    float distance = 100.f;
    RE::NiPoint2 angle = {0, 0};
    RE::NiPoint2 position = {0, 0};
    inline static Manager* singleton = NULL;

    static inline bool doRotate = false;
    static inline bool doTranslate = false;
    static inline bool doTranslateZ = false;

    void UpdateObjectTransform(RE::TESObjectREFR* obj, RE::NiPoint3& rayPosition);

    void EnableRotate(RE::ButtonEvent* button) {
    }

public:

    bool GetDoRotate() {
        return doRotate;
    }

    bool GetDoTranslate() {
        return doTranslate ;
    }

    bool GetTranslateZ() {
        return doTranslateZ;
    }

    Manager() { 
        auto input = InputManager::GetSingleton();
        input->AddSink("Rotation", [](RE::ButtonEvent* button) { 
            if (button->IsDown()) {
                doRotate = true;
            } else if (button->IsUp()) {
                doRotate = false;
            }
        });

        input->AddSink("Translation", [](RE::ButtonEvent* button) {
            if (button->IsDown()) {
                doTranslate = true;
            } else if (button->IsUp()) {
                doTranslate = false;
            }
        });

        input->AddSink("ZTranslation", [](RE::ButtonEvent* button) {
            if (button->IsDown()) {
                doTranslateZ = true;
            } else if (button->IsUp()) {
                doTranslateZ = false;
            }
        });

        input->AddSink("MoveObjectCloser", [](RE::ButtonEvent* button) {
            Manager::GetSingleton()->IncrementDistance(3.f);
        });

        input->AddSink("MoveObjectFruther", [](RE::ButtonEvent* button) {
            Manager::GetSingleton()->IncrementDistance(-3.f);
        });

        input->AddSink("RotateXPlus",
        [this](RE::ButtonEvent* button) { 
            if (button->IsHeld()) {
                angle.x += 10.f;
            }
        });
        input->AddSink("RotateXMinus", [this](RE::ButtonEvent* button) {
            if (button->IsHeld()) {
                angle.x -= 10.f;
            }
        });
        input->AddSink("RotateXPlus", [this](RE::ButtonEvent* button) {
            if (button->IsHeld()) {
                angle.x += 10.f;
            }
        });
    }
    static Manager* GetSingleton() {
        if (!singleton) {
            singleton = new Manager();
        }
        return singleton;
    }
    void SetGrabbing(bool value, RE::TESObjectREFRPtr ref);
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